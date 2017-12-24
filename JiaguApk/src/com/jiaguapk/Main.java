package com.jiaguapk;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.zip.Adler32;

public class Main {

	/**
	  --------------------
	 |                    |
	 |       脱壳用                            |
	 |    classes.dex     |
	  --------------------
	 |       目标源                           |
	 |    TargetApk.zip   |
	  --------------------
	 | TargetApk.zip文件大小   |
	  --------------------
	  
	 */
	public static void main(String[] args) throws Exception {
		
		if(args.length != 2){
			System.out.println("Error! Please input 2 parameters!");
			return ;
		}
		
		//参数1：脱壳classes.dex
		//参数2：TargetApk.zip
		File tuokeDexFile = new File(args[0]);
		File targetFile = new File(args[1]);
		
		byte[] tuokeDexFileArray = readFileBytes(tuokeDexFile);
		byte[] targetFileArray = encrypt(readFileBytes(targetFile));
		int tuokeDexFileLen = tuokeDexFileArray.length;
		int targetFileLen = targetFileArray.length;
		int totalLen = targetFileLen + tuokeDexFileLen + 4;
		byte[] newdex = new byte[totalLen];
		//添加脱壳classes.dex文件
		System.arraycopy(tuokeDexFileArray, 0, newdex, 0, tuokeDexFileLen);
		//添加目标TargetApk.zip文件
		System.arraycopy(targetFileArray, 0, newdex, tuokeDexFileLen, targetFileLen);
		//添加目标zip文件大小
		System.arraycopy(intToByte(targetFileLen), 0, newdex, tuokeDexFileLen + targetFileLen, 4);
		//修改Dex file size文件头
		fixFileSizeHeader(newdex);
		//修改Dex SHA1文件头
		fixSHA1Header(newdex);
		//修改Dex CheckSum文件头
		fixCheckSumHeader(newdex);
		
		File file = new File("classes.dex");
		if(!file.exists()){
			file.createNewFile();
		}
		
		FileOutputStream out = new FileOutputStream(file);
		out.write(newdex);
		out.flush();
		out.close();
		
		System.out.println("Enforce apk successfully!");
		

	}
	
	private static byte[] encrypt(byte[] srcdata){  
        for(int i = 0;i<srcdata.length;i++){  
            srcdata[i] = (byte)(~srcdata[i]);  
        }  
        return srcdata;  
    } 
	
	/** 
     * 修改dex头，CheckSum 校验码 
     * @param dexBytes 
     */  
    private static void fixCheckSumHeader(byte[] dexBytes) {  
        Adler32 adler = new Adler32();  
        adler.update(dexBytes, 12, dexBytes.length - 12);//从12到文件末尾计算校验码  
        long value = adler.getValue();  
        int va = (int) value;  
        byte[] newcs = intToByte(va);  
        //高位在前，低位在前掉个个  
        byte[] recs = new byte[4];  
        for (int i = 0; i < 4; i++) {  
            recs[i] = newcs[newcs.length - 1 - i];  
        }  
        System.arraycopy(recs, 0, dexBytes, 8, 4);//效验码赋值（8-11）  
    }  
  
  
    /** 
     * int 转byte[] 
     * @param number 
     * @return 
     */  
    public static byte[] intToByte(int number) {  
        byte[] b = new byte[4];  
        for (int i = 3; i >= 0; i--) {  
            b[i] = (byte) (number % 256);  
            number >>= 8;  
        }  
        return b;  
    }  
  
    /** 
     * 修改dex头 sha1值 
     * @param dexBytes 
     * @throws NoSuchAlgorithmException 
     */  
    private static void fixSHA1Header(byte[] dexBytes)  
            throws NoSuchAlgorithmException {  
        MessageDigest md = MessageDigest.getInstance("SHA-1");  
        md.update(dexBytes, 32, dexBytes.length - 32);//从32为到结束计算sha--1  
        byte[] newdt = md.digest();  
        System.arraycopy(newdt, 0, dexBytes, 12, 20);//修改sha-1值（12-31）  
        //输出sha-1值，可有可无  
        String hexstr = "";  
        for (int i = 0; i < newdt.length; i++) {  
            hexstr += Integer.toString((newdt[i] & 0xff) + 0x100, 16)  
                    .substring(1);  
        }  
    }  
  
    /** 
     * 修改dex头 file_size值 
     * @param dexBytes 
     */  
    private static void fixFileSizeHeader(byte[] dexBytes) {  
        //新文件长度  
        byte[] newfs = intToByte(dexBytes.length);  
        byte[] refs = new byte[4];  
        //高位在前，低位在前掉个个  
        for (int i = 0; i < 4; i++) {  
            refs[i] = newfs[newfs.length - 1 - i];  
        }  
        System.arraycopy(refs, 0, dexBytes, 32, 4);//修改（32-35）  
    }  
  
  
    /** 
     * 以二进制读出文件内容 
     * @param file 
     * @return 
     * @throws IOException 
     */  
    private static byte[] readFileBytes(File file) throws IOException {  
        byte[] arrayOfByte = new byte[1024];  
        ByteArrayOutputStream localByteArrayOutputStream = new ByteArrayOutputStream();  
        FileInputStream fis = new FileInputStream(file);  
        while (true) {  
            int i = fis.read(arrayOfByte);  
            if (i != -1) {  
                localByteArrayOutputStream.write(arrayOfByte, 0, i);  
            } else {  
                return localByteArrayOutputStream.toByteArray();  
            }  
        }  
    }

}
