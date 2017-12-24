#!/usr/bin/env python
#coding:utf-8
import subprocess
import shutil
from  xml.dom import  minidom
import zipfile 
import os
import re
import glob
import sys
import codecs
import random
import string
import time

from elf_header import ELF


'''
一、针对目标app不存在自定义application的情况
	1.反编译目标app
	apktool.bat d Target.apk
	2.检测manifest文件是否有自定义的Application，并假设没有自定义Application
	3.如果没有自定义Application，则复制smali文件夹，跟反编译后的app下的smali合并: cp -rf smali Target/
	4.修改manifest文件，将自定义Application设定为“org.hackcode.ProxyApplication”
	5.重打包目标app
	6.提取重打包后的apk中的classes.dex文件，并压缩为TargetApk.zip文件，并将重打包的app命名为Target.modified.apk
	7.合并TuokeApk项目下的classes.dex和TargetApk.zip(加固),生成classes.dex
	8.将合并生成的新classes.dex文件与Target.modified.apk中的classes.dex替换
	9.复制TuokeApk项目下的lib目录下的所有文件和文件夹到目标app中
	10.将修改后的app重压缩成zip文件
	11.签名
'''

def un_zip(file_name):  
    """unzip zip file"""  
    zip_file = zipfile.ZipFile(file_name)  
    if os.path.isdir(file_name + "_files"):  
        pass  
    else:  
        os.mkdir(file_name + "_files")  
    for names in zip_file.namelist():  
        zip_file.extract(names,file_name + "_files/")  
    zip_file.close() 
    return file_name + "_files"

def zip_dir(dirname,zipfilename):
    filelist = []
    if os.path.isfile(dirname):
        filelist.append(dirname)
    else :
        for root, dirs, files in os.walk(dirname):
            for name in files:
                filelist.append(os.path.join(root, name))

    zf = zipfile.ZipFile(zipfilename, "w", zipfile.zlib.DEFLATED)
    for tar in filelist:
        arcname = tar[len(dirname):]
        zf.write(tar,arcname)
    zf.close()

def recompile_TuokeApk_Project(application_name):
	'''
	1.修改 String appClassName = "com.targetapk.MyApplication";
	2.重新编译
	'''
	file_path = 'TuokeApk/app/src/main/java/org/hackcode/ProxyApplication.java'
	new_file_path = 'TuokeApk/app/src/main/java/org/hackcode/ProxyApplication2.java'
	file_in = open(file_path)
	file_out = open(new_file_path, 'w')
	while 1:
		line = file_in.readline()
		if not line:
			break
		pattern = re.compile(r'.*String.*appClassName.*=.*\".*\";.*')
		if re.search(pattern, line):
			print '[+] Find \"String appClassName = ...\", replace it with \"' + application_name + '\"'
			file_out.write('\t\t\tString appClassName = \"' + application_name + '\";\n')
		else:
			file_out.write(line)
	file_in.close()
	file_out.close()

	os.remove(file_path)
	os.rename(new_file_path, file_path)

	# 重新编译TuokeApk工程
	os.chdir('TuokeApk/')
	
	subprocess.Popen(["gradle","clean"], shell=True, stdout=subprocess.PIPE).stdout.read()
	out = subprocess.Popen(["gradle","build"], shell=True, stdout=subprocess.PIPE).stdout.read()
	if out.find('BUILD SUCCESSFUL') < 0:
		print 'Build error!'
		return False
	print '[+] Rebuild TuokeApk project successfully!'
	os.chdir('../')

	return True

def remove_without_exception(item, type):
	if type == 'd':
		try:
			shutil.rmtree(item)
		except Exception as e:
			pass
	else:
		try:
			os.remove(item)
		except Exception as e:
			pass

def clean():
	remove_without_exception('Target', 'd')
	remove_without_exception('Target.modified.apk_files', 'd')
	remove_without_exception('Target.apk', 'f')
	remove_without_exception('Target.modified.apk', 'f')
	remove_without_exception('Target.modified.2.apk', 'f')
	remove_without_exception('classes.dex', 'f')
	remove_without_exception('TargetApk.zip', 'f')
	remove_without_exception('tuoke.dex', 'f')

	os.chdir('TuokeApk/')
	subprocess.Popen(["gradle","clean"], shell=True, stdout=subprocess.PIPE).stdout.read()
	os.chdir('../')

def genRandomStr(length):
    chars=string.ascii_letters+string.digits
    return ''.join([random.choice(chars) for i in range(length)])#得出的结果中字符会有重复的

def modify_ehdr_and_delete_shdr(apk_dir):
	'''
	修改ELF header(e_shoff和e_shnum属性)和删除section header table
	TODO: 指定目标so文件
	'''
	for root, dirs, files in os.walk(apk_dir):
		for name in files:
			filepath = root + os.path.sep + name
			if filepath.endswith('libhackcodejiagu.so'):
				print '    - Modifying \"', filepath, '\" ELF header...'
				dex = ELF(filepath)
				file_size = os.path.getsize(filepath)
				shdr_offset = dex.elf32_Ehdr.e_shoff
				shdr_size = dex.elf32_Ehdr.e_shnum * dex.elf32_Ehdr.e_shentsize

				src_file = file(filepath, 'rb')
				dst_file = file(filepath + '2', 'wb')
				# 1.破坏ELF Header
				dst_file.write(src_file.read(32)) # 保存e_shoff之前的内容
				src_file.read(4)
				dst_file.write(genRandomStr(4)) # 修复e_shoff

				dst_file.write(src_file.read(12)) # 保存e_shoff到e_shnum之间的内容
				src_file.read(2)
				dst_file.write(genRandomStr(2)) # 修复e_shnum				

				# 2.删除section header table
				#读取section header table之前的内容
				dst_file.write(src_file.read(shdr_offset - 50))

				#读取section header table之后的内容
				src_file.seek(shdr_offset + shdr_size, 0)
				dst_file.write(src_file.read())

				src_file.close()
				dst_file.close()

				shutil.move(filepath + '2', filepath)

def main(filepath = None):
	clean()
	if filepath:
		input_filename = filepath
	else:
		input_filename = sys.argv[1]
	shutil.copyfile(input_filename, 'Target.apk')

	# Step1: 反编译目标app
	out = subprocess.Popen('apktool.bat d Target.apk', stdout=subprocess.PIPE).stdout.read()

	if out.find('error') > 0 or out.find('exception') > 0:
		print '[Error] apktool decompiled error!'
		return
	print '[+] Apktool decompiled Target.apk successfully!'


	# Step2: 检测manifest文件是否有自定义的Application
	doc = minidom.parse('Target/AndroidManifest.xml')
	root = doc.documentElement
	application_node = root.getElementsByTagName('application')[0]
	applicationName = application_node.getAttribute('android:name')

	packageName = root.getAttribute('package')
	if applicationName:
		if not applicationName.startswith(packageName) and applicationName.startswith('.'):
			applicationName = packageName + applicationName
		print '[+] Target app\'s Application: ', applicationName
		# Step3: 修改JiguApk工程中ProxyApplication中的applicationName变量为目标app的Application名称
		recompile_TuokeApk_Project(applicationName)
	else:
		print '[+] Target.apk has no self-defined Application!'
		applicationName = 'com.targetapk.MyApplication'
		recompile_TuokeApk_Project(applicationName)
		# Step3: 复制smali文件夹，跟反编译后的app下的smali合并
		print '[+] Copy smali folder into Target folder...'
		out = subprocess.Popen('cp -rf smali Target/', stdout=subprocess.PIPE).stdout.read()

	# Step4: 修改manifest文件，将自定义Application设定为“org.hackcode.ProxyApplication”
	print '[+] Modified AndroidManifest.xml...'
	application_node.setAttribute('android:name', 'org.hackcode.ProxyApplication')
	file_handle = codecs.open('Target/AndroidManifest.xml','w', 'utf-8')
	root.writexml(file_handle)
	file_handle.close()

	# Step5: 重打包目标app
	out = subprocess.Popen('apktool.bat b Target', stdout=subprocess.PIPE).stdout.read()
	if out.find('error') > 0 or out.find('exception') > 0:
		print '[Error] apktool recompiled error!'
		return
	print '[+] Apktool recompiled Target successfully!'

	# Step6: 将重打包的app命名为Target.modified.apk,并提取重打包后的apk中的classes.dex文件，并压缩为TargetApk.zip文件
	print '[+] Rename target app: \"Target.modified.apk\"'
	shutil.copyfile('Target/dist/Target.apk', 'Target.modified.apk')
	extracted_dir = un_zip('Target.modified.apk')

	print '[+] Extracted classes.dex from Target.modifed.apk into TargetApk.zip'
	shutil.copyfile(extracted_dir + '/classes.dex', 'classes.dex')
	
	# 写入classes.dex
	f = zipfile.ZipFile('TargetApk.zip', 'w', zipfile.ZIP_DEFLATED)
	f.write('classes.dex')
	f.close()
	os.remove('classes.dex')
	
	# Step7: 合并TuokeApk/bin/classes.dex和TargetApk.zip(加固),生成classes.dex
	shutil.copyfile('TuokeApk/app/build/intermediates/transforms/dex/release/folders/1000/1f/main/classes.dex', 'tuoke.dex')
	subprocess.Popen('java -jar JiaguApk.jar tuoke.dex TargetApk.zip', stdout=subprocess.PIPE).stdout.read()

	# Step8: 将合并生成的新classes.dex文件与Target.modified.apk中的classes.dex替换
	print '[+] Replace \"%s\" with \"classes.dex\"' % (extracted_dir + '/classes.dex', )
	shutil.copyfile('classes.dex', extracted_dir + '/classes.dex')

	# Step9: 复制TuokeApk/libs目录下的所有文件和文件夹到目标app中
	print '[+] Copying TuokeApk/app/build/intermediates/ndk/release/lib/...'
	if not os.path.exists(extracted_dir + '/lib/'):
		os.mkdir(extracted_dir + '/lib/')
		for item in os.listdir('TuokeApk/app/build/intermediates/ndk/release/lib/'):
			if not os.path.exists(extracted_dir + '/lib/' + item):
				shutil.copytree('TuokeApk/app/build/intermediates/ndk/release/lib/' + item, extracted_dir + '/lib/' + item)
			else:
				shutil.copyfile('TuokeApk/app/build/intermediates/ndk/release/lib/' + item + '/libhackcodejiagu.so', extracted_dir + '/lib/' + item + '/libhackcodejiagu.so')
	else:
		for item in os.listdir(extracted_dir + '/lib/'):
			shutil.copyfile('TuokeApk/app/build/intermediates/ndk/release/lib/' + item + '/libhackcodejiagu.so', extracted_dir + '/lib/' + item + '/libhackcodejiagu.so')

	# 破坏SO文件的ELF头部（删除 ELF header）
	modify_ehdr_and_delete_shdr(extracted_dir)

	# Step10: 将修改后的app重压缩成zip文件
	print '[+] Compress %s folder into Target.modified.2.apk' % extracted_dir
	zip_dir(extracted_dir, 'Target.modified.2.apk')

	# Step11: 签名
	print '[+] Signning...'
	output_filename = input_filename[:input_filename.rfind('apk')] + 'signed.apk'
	out = subprocess.Popen('java -jar sign/signapk.jar sign/testkey.x509.pem sign/testkey.pk8 Target.modified.2.apk ' + output_filename, stdout=subprocess.PIPE).stdout.read()
	clean()

if __name__ == '__main__':
	start = time.time()
	main()
	end = time.time()
	print "Total time running %s seconds" %(str(end - start))
	
	 

	
	










