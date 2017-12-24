#!/usr/bin/env python
#coding:utf-8
import binascii

class ELF(object):
    """docstring for ELF"""
    def __init__(self, filepath):
        super(ELF, self).__init__()
        self.filepath = filepath
        self.elf32_Ehdr = Elf32_Ehdr()

        self.initELFHeader()

    def initELFHeader(self):
        f = open(self.filepath, "rb")
        self.f = f
        # unsigned char	e_ident[EI_NIDENT];
        f.seek(0, 0)
        self.elf32_Ehdr.e_ident = e_ident()
        self.elf32_Ehdr.e_ident.file_identification = f.read(4)
        self.elf32_Ehdr.e_ident.ei_class = int(binascii.b2a_hex(f.read(1)), 16)
        self.elf32_Ehdr.e_ident.ei_data = int(binascii.b2a_hex(f.read(1)), 16)
        self.elf32_Ehdr.e_ident.ei_version = int(binascii.b2a_hex(f.read(1)), 16)
        self.elf32_Ehdr.e_ident.ei_osabi = int(binascii.b2a_hex(f.read(1)), 16)
        self.elf32_Ehdr.e_ident.ei_abiversion = int(binascii.b2a_hex(f.read(1)), 16)
        self.elf32_Ehdr.e_ident.ei_pad = binascii.b2a_hex(f.read(6))
        self.elf32_Ehdr.e_ident.ei_nident = int(binascii.b2a_hex(f.read(1)), 16)

        # Elf32_Half	e_type;
        f.seek(16, 0)
        self.elf32_Ehdr.e_type = int(binascii.b2a_hex(f.read(2)).decode('hex')[::-1].encode('hex'), 16)

        # Elf32_Half	e_machine;
        f.seek(18, 0)
        self.elf32_Ehdr.e_machine = int(binascii.b2a_hex(f.read(2)).decode('hex')[::-1].encode('hex'), 16)

        # Elf32_Word	e_version;
        f.seek(20, 0)
        self.elf32_Ehdr.e_version = int(binascii.b2a_hex(f.read(4)).decode('hex')[::-1].encode('hex'), 16)

        # Elf32_Addr	e_entry;
        f.seek(24, 0)
        self.elf32_Ehdr.e_entry = int(binascii.b2a_hex(f.read(4)).decode('hex')[::-1].encode('hex'), 16)

        # Elf32_Off	e_phoff;
        f.seek(28, 0)
        self.elf32_Ehdr.e_phoff = int(binascii.b2a_hex(f.read(4)).decode('hex')[::-1].encode('hex'), 16)

        # Elf32_Off	e_shoff;
        f.seek(32, 0)
        self.elf32_Ehdr.e_shoff = int(binascii.b2a_hex(f.read(4)).decode('hex')[::-1].encode('hex'), 16)

        # Elf32_Word	e_flags;
        f.seek(36, 0)
        self.elf32_Ehdr.e_flags = int(binascii.b2a_hex(f.read(4)).decode('hex')[::-1].encode('hex'), 16)

        # Elf32_Half	e_ehsize;
        f.seek(40, 0)
        self.elf32_Ehdr.e_ehsize = int(binascii.b2a_hex(f.read(2)).decode('hex')[::-1].encode('hex'), 16)

        # Elf32_Half	e_phentsize;
        f.seek(42, 0)
        self.elf32_Ehdr.e_phentsize = int(binascii.b2a_hex(f.read(2)).decode('hex')[::-1].encode('hex'), 16)

        # Elf32_Half	e_phnum;
        f.seek(44, 0)
        self.elf32_Ehdr.e_phnum = int(binascii.b2a_hex(f.read(2)).decode('hex')[::-1].encode('hex'), 16)

        # Elf32_Half	e_shentsize;
        f.seek(46, 0)
        self.elf32_Ehdr.e_shentsize = int(binascii.b2a_hex(f.read(2)).decode('hex')[::-1].encode('hex'), 16)

        # Elf32_Half	e_shnum;
        f.seek(48, 0)
        self.elf32_Ehdr.e_shnum = int(binascii.b2a_hex(f.read(2)).decode('hex')[::-1].encode('hex'), 16)

        # Elf32_Half	e_shstrndx;
        f.seek(50, 0)
        self.elf32_Ehdr.e_shstrndx = int(binascii.b2a_hex(f.read(2)).decode('hex')[::-1].encode('hex'), 16)

class Elf32_Ehdr(object):
    """docstring for Elf32_Ehdr"""
    def __init__(self):
        super(Elf32_Ehdr, self).__init__()
        self.e_ident = None
        self.e_type = None
        self.e_machine = None
        self.e_version = None
        self.e_entry = None
        self.e_phoff = None
        self.e_shoff = None
        self.e_flags = None
        self.e_ehsize = None
        self.e_phentsize = None
        self.e_phnum = None
        self.e_shentsize = None
        self.e_shnum = None
        self.e_shstrndx = None

class e_ident(object):
    """docstring for e_ident"""
    def __init__(self):
        super(e_ident, self).__init__()
        self.file_identification = None
        self.ei_class = None
        self.ei_data = None
        self.ei_version = None
        self.ei_osabi = None
        self.ei_abiversion = None
        self.ei_pad = None
        self.ei_nident = None

    def __str__(self):
        return 'e_ident=[file_identification=%s, ei_class=%d, ei_data=%d, ei_version=%d, ei_osabi=%d, ei_abiversion=%d, ei_pad=%s, ei_nident=%d]' % (
        self.file_identification, self.ei_class, self.ei_data, self.ei_version, self.ei_osabi, self.ei_abiversion, self.ei_pad, self.ei_nident)