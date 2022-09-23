#!/usr/bin/env python3
'''
/*
 * Excelfore Universal IPC and Configuration code generator
 * Copyright (C) 2020 Excelfore Corporation (https://excelfore.com)
 *
 * This file is part of Excelfore-unipac.
 *
 * Excelfore-unipac is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * Excelfore-unipac is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Excelfore-unipac.  If not, see
 * <https://www.gnu.org/licenses/old-licenses/gpl-2.0.html>.
 */
'''
import re
import sys, os
import getopt
import logging
from io import StringIO

logging.basicConfig()
logger=logging.getLogger('up_genconf')

def find_brapair(v, a):
    pv=None
    if a=='(': b=')'
    elif a=='[': b=']'
    elif a=='{': b='}'
    if not b: return v, 0
    if len(v)<2: return v, 0
    if v[0]!=a: return v, 0
    c=0
    for i in range(len(v[1:])):
        if v[1+i]==b:
            if c==0: return v[1:1+i], i+2
            c-=1
            continue
        if v[1+i]==a: c+=1
    return v, 0

# return (next string, repeat number, cosumed characters)
def num_in_bracket(v):
    vs,n=find_brapair(v, '[')
    if n==0: return v,1,0
    try:
        a=int(vs)
    except ValueError:
        try:
            a=eval(vs)
        except ValueError:
            return v,1,0
    return v[n:],a,n

def find_quote(v):
    if v[0]!='"': return 0
    nn=0
    while True:
        qn=v[nn+1:].find('"')
        if qn<0: return 0
        if v[nn+qn]!='\\': return nn+qn+1
        nn=nn+qn+1

def find_delimiters(v, td='{,:', bd=':,}'):
    consumed_chars=0
    tdelim=''
    bdelim=''
    vl=len(v)
    v=v.lstrip()
    consumed_chars+=vl-len(v)
    qn=find_quote(v)
    for i in td:
        if v[0]==i:
            tdelim=v[0]
            consumed_chars+=1
            v=v[1:]
            break;
    if not bd:
        return v,consumed_chars,tdelim,bdelim
    nl=[]
    for i in bd:
        n=v.find(i)
        if n>qn: nl.append(n)
    if nl:
        n=min(nl)
        bdelim=v[n]
        consumed_chars+=n
        v=v[:n]
    else:
        consumed_chars+=len(v)
    v=v.rstrip()
    return v,consumed_chars,tdelim,bdelim

class ValueTypes(dict):
    STRING_TYPE_BASE=10000
    def __init__(self):
        vt=['invalid',
            'int8_t','int16_t','int32_t','int64_t',
            'uint8_t','uint16_t','uint32_t','uint64_t',
            'float','double','char','bool']
        rv={}
        for i,v in enumerate(vt):
            rv[v]=-(i+1);
        super().__init__(rv)

value_types=ValueTypes()

class ValueAtom(tuple):
    '''
    [,]12Y[,]	int8_t
    [,]1234S[,]	int16_t
    [,]1234[,]	int32_t
    [,]1234L[,]	int64_t
    [,]0x2AY[,]	uint8_t
    [,]0x2AS[,]	uint16_t
    [,]0x2A[,]	uint32_t
    [,]0x2AL[,]	uint64_t
    [:]2A[:]	uint8_t(hex format)
    [:]2A3B[:]	uint16_t(hex format)
    [,]'a'[,]	char
    [,]"abcd"[,]	stringNUM(NUM is the length not including last null)
    [,]"abcd"[N][,]	stringNUM(NUM becomes N-1)
    [,]1.23[,]	double
    [,]1.23F[,]	float
    [,]true/false[,]	bool
    '''
    STRTPREF='upstring'
    STRTPREFLEN=8
    # a tupple:(type, value) is created from a string
    def __new__(cls, v, bw=0, unsigned=None, length=0):
        cls.consumed_chars=0
        if not v or v.__class__!=str:
            return super().__new__(cls)
        v,cls.consumed_chars,tdelim,bdelim=find_delimiters(v)

        if v[0]=="'":
            if v[2]!="'":
                if v[1]=="\\" and v[2]=="0":
                    return super().__new__(cls, ('char', '\0'))
                raise ValueError("not a char:%s" % v)
            return super().__new__(cls, ('char', v[1]))
        if v[0]=='"':
            ssize=0
            if v[-1]==']':
                n=v.rfind('[')
                if n<0: ValueError("bad format of string:%s" % v)
                ssize=int(v[n+1:-1])
                v=v[:n]
            if v[-1]!='"': ValueError("string has no end of \":%s" % v)
            v=v[1:-1]
            if length==0:
                length=ssize-1 if ssize else len(v)
            if len(v)>length: v=v[:length]
            return super().__new__(cls, ('%s%d' % (ValueAtom.STRTPREF,length), v))
        if v.find(".")>=0:
            if v[len(v)-1]=='F':
                rv=float(v[:-1])
                return super().__new__(cls, ('float', rv))
            else:
                rv=float(v)
                return super().__new__(cls, ('double', rv))
        if v=="true":
            return super().__new__(cls, ('bool', True))
        if v=="false":
            return super().__new__(cls, ('bool', False))

        if v[len(v)-1]=='Y':
            v=v[:-1]
            if bw==0: bw=8
        elif v[len(v)-1]=='S':
            v=v[:-1]
            if bw==0: bw=16
        elif v[len(v)-1]=='L':
            v=v[:-1]
            if bw==0: bw=64

        if tdelim==':' or bdelim==':':
            rv=int(v, 16)
            if unsigned==None: unsigned=True
            if bw==0:
                if cls.consumed_chars<4:
                    bw=8
                else:
                    bw=16
        else:
            try:
                rv=int(v, 0)
            except ValueError:
                rv=int(v, 16)

        if unsigned==None:
            if v.find('0x')==0: unsigned=True
        pf="u" if unsigned else ""
        if bw:
            return super().__new__(cls, ('%sint%d_t' % (pf, bw), rv))
        if rv<-2147483648 or rv>=2147483648:
            return super().__new__(cls, ('%sint64_t' % pf, rv))
        return super().__new__(cls, ('%sint32_t' % pf, rv))

    def __init__(self, v, **argv):
        self.consumed_chars=self.__class__.consumed_chars
        self.str_format=None

    def __str__(self):
        if self[0]=='bool': return str(self[1]).lower()
        if self[0]=='char':
            if self[1]=='\0':
                return "'\\0'"
            else:
                return "'%s'" % self[1]
        if self[0].find(ValueAtom.STRTPREF)==0:
            return '"%s"' % self[1]
        if self[0]=='uint8_t' and self.str_format=='hex':
            return '%02X' % self[1]
        if self[0]=='uint16_t' and self.str_format=='hex':
            return '%04X' % self[1]
        return str(self[1])

    def sizeof(self):
        r=re.match(r"[^0-9]*([0-9]+)", self[0])
        if r:
            if self[0].find(ValueAtom.STRTPREF)==0: return int(r.group(1))+1
            return int(r.group(1))//8
        if self[0]=="bool": return 1
        if self[0]=="char": return 1
        if self[0]=="double": return 8
        if self[0]=="float": return 4
        raise ValueError('unknonw type %s' % self[0])

    def get_string_types(self):
        if self[0].find(ValueAtom.STRTPREF)==0: return [self[0]]
        return []

class ValueUnit(list):
    #VUnit=VAtom0,VAtom1,VAtom2,,,
    #VUnit=VAtom0,{VAtom1,VAtom2},Vatom3,, = VAtom0, VUnit0, Vatom3,,,
    #VUnit=VAtom0,[n]VAtom1,VAtom2,,,=VAtom0,VAtom1,VAtom1,,,VAtom1,VAtom2,,,
    def __init__(self, v=[], nofe=0, str_delimiter=',', str_format=None, struct=None):
        if self.__class__==ValueUnit:
            self.nofe1=0
            self.str_delimiter=str_delimiter
            self.str_format=str_format
            self.array=False
            self.str_nobraces=False
        if not v or v.__class__!=str:
            if v.__class__==ValueUnit:
                self.nofe1=v.nofe1
                self.str_delimiter=v.str_delimiter
                self.str_format=v.str_format
                self.array=v.array
            return super().__init__(v)

        vv=v;
        res=[]
        self.consumed_chars=0
        repeatn=0
        while len(v)>self.consumed_chars:
            # skip leading ','
            vv,n,tdelim,bdelim=find_delimiters(v[self.consumed_chars:], td=",", bd=None)
            self.consumed_chars+=n
            # get N in [N]
            vv,repeatn,n=num_in_bracket(vv)
            prefix_N=True if n>0 else False
            self.consumed_chars+=n
            # get inside of {...}
            vvp,n=find_brapair(vv, '{')
            if n:
                # for inside {}, make ValueUnit
                self.consumed_chars+=n
                vu=ValueUnit(vvp)
                if not vu: break
                for i in range(repeatn):
                    res.append(ValueUnit(vvp))
            else:
                # no {}, make ValueAtom
                vu=ValueAtom(vvp)
                if not vu: break
                self.consumed_chars+=vu.consumed_chars
                for i in range(repeatn):
                    res.append(ValueAtom(vvp))

        self.nofe1=max(nofe, len(res))
        if len(res)>1 and repeatn>1:
            self.array=True
        elif len(res)>=1 and repeatn>=1 and prefix_N:
            self.array=True
        super().__init__(res)
        if not struct and not self.array and len(res)>1:
            self.array=self.children_same_type()

    def children_same_type(self):
        if len(self)<=1: return False
        k=None
        for i in self:
            if k==None:
                if i.__class__==ValueAtom:
                    k=i[0]
                    continue
                # structure's array is defined only by 'repeatn', so this is not struct
                if not i.children_same_type(): return False
                k=len(i)
                continue
            if i.__class__==ValueAtom:
                if k.__class__ != str : return False
                if k==i[0]: continue # the same ValueAtom[0] type
                if k.find(ValueAtom.STRTPREF)==0 and i[0].find(ValueAtom.STRTPREF)==0: continue
                return False
            if not i.children_same_type(): return False
            if k==len(i): continue
            return False
        return True

    def get_index_vtype(self, index=0):
        if len(self)<1: return None
        v=self[index]
        while v.__class__!=ValueAtom: v=v[0]
        return v[0]

    def get_first_vtype(self):
        return self.get_index_vtype(index=0)

    def get_first_atom(self):
        v=self
        while v.__class__!=ValueAtom: v=v[0]
        return v

    # for string array check all indexes and get the maximum stringN,
    # for the others, the same as get_first_vtype
    def get_first_smax_vtype(self):
        r=self.get_first_vtype()
        if r.find(ValueAtom.STRTPREF)!=0 or not self.array: return r
        vs=int(r[ValueAtom.STRTPREFLEN:])
        for i in range(len(self)-1):
            r=self.get_index_vtype(index=i+1)
            vs=max(vs,int(r[ValueAtom.STRTPREFLEN:]))
        return '%s%d' % (ValueAtom.STRTPREF, vs)

    def __str__(self):
        rs=[]
        for i in self:
            i.str_format=self.str_format
            rs.append(str(i))
        res=self.str_delimiter.join(rs)
        if self.str_nobraces:
            return "%s" % (res)
        else:
            return "{%s}" % (res)

    def __len__(self):
        return max(self.nofe1, super().__len__())

    def number_of_tuples(self):
        rv=0
        for i in self:
            if i.__class__==ValueAtom:
                rv+=1
            else:
                rv+=i.number_of_tuples()
        return rv

    def get_string_types(self):
        if len(self)<1: return []
        res=[]
        if self.array and self[0].__class__==ValueAtom and self[0][0].find(ValueAtom.STRTPREF)==0:
            return [self.get_first_smax_vtype()]
        for v in self:
            res+=v.get_string_types()
        return res

    def max_len_in_children(self):
        res=1
        for i in self:
            if i.__class__==ValueAtom: continue
            res=max(res, len(i))
        return res;

class StructDefinitions(dict):
    def register_struct(self, struct, values, fnames):
        stv=[]
        if struct in self: return
        for i,v in enumerate(values):
            if fnames and len(fnames)>i:
                fn=fnames[i]
            else:
                fn="f%d" % i
            if v.__class__==ValueUnit:
                n1=len(v)
                n2=1
                stv.append((v.get_first_smax_vtype(), n1, n2, fn))
            else:
                stv.append((v[0], 1, 1, fn))
        self[struct]=stv

    @classmethod
    def get_vtype_numstr(cls, v):
        if v.find(ValueAtom.STRTPREF)!=0: return "VT_%s" % v.upper()
        return "%d" % (int(v[ValueAtom.STRTPREFLEN:])+ValueTypes.STRING_TYPE_BASE)

    @classmethod
    def get_sizeof_str(cls, v):
        if v.find(ValueAtom.STRTPREF)!=0: return "sizeof(%s)" % v
        return "%d" % (int(v[ValueAtom.STRTPREFLEN:])+1)

class SpaceInQuote(object):
    rpmark='X*&$Z'
    @classmethod
    def replace_spaces(cls, line):
        indq=False
        insq=False
        bslash=False
        repl=False
        nline=[]
        for i in line:
            if i=='\\':
                bslash=True
                nline.append('\\')
                continue
            while True:
                if i!='"' and i!="'":
                    if (indq or insq) and i==' ':
                        nline.append(cls.rpmark)
                        repl=True
                    else:
                        nline.append(i)
                    break
                else:
                    if bslash:
                        nline.append(i)
                        break
                    if i=='"':
                        indq=not indq
                    else:
                        insq=not insq
                    nline.append(i)
                    break

            bslash=False
        return ''.join(nline), repl

    @classmethod
    def back_to_spaces(cls, line, repl=True):
        if not repl: return line
        return line.replace(cls.rpmark, ' ')

class ScanConfigFile(object):
    '''
    scan the configuration file, and create self.ditems.
    self.ditems is a dictonary indexed by 'vname':variable name.
    the element of 'self.ditems' has a dictionary as follows:
    {'struct':struct, 'values':values, 'pi':index_pointer}
    * struct is None for a simple variable, and 'struct name' for struct variable
    * number_of_element is the number of element of the variable, 1 for non-array type
    * values is a list which includes all fields of the variable.
    * index_pointer is internaly used pointer to index the posion of currently updating element.
      it is used only for array type and when it is updated without explicit index number.

    v['values'] has a ValueUnit instance
    '''
    def __init__(self, fname="defaults.cfg", tfname=None, pf=''):
        self.pf=pf
        self.in_header_addition=False
        inf=self.file_preproc(fname)
        if tfname:
            self.toutf=open(tfname, "w")
        else:
            self.toutf=None
        recomment=re.compile(r"^([^#]*)#(.*)")
        recomps=[]
        recomps.append(re.compile(r"^\s*(\S*):fname\s+(\S+)"))
        recomps.append(re.compile(r"^(.*)\s+([IRWP]+)\s*$"))
        recomps.append(re.compile(r"^\s*(\S*)-(\S[^[ ]*)\[(\d*)\]\.?(\S*)\s+(\S+)"))
        recomps.append(re.compile(r"^\s*(\S*)-([^.\s]*)\.?(\S*)\s+(\S+)"))
        recomps.append(re.compile(r"^\s*(\S[^[ ]*)\[(\d*)\]\s+(\S+)"))
        recomps.append(re.compile(r"^\s*(\S*)\s+(\S+)"))

        self.struct_defs=StructDefinitions()
        self.string_types_set=set()
        self.thirdelement_type_set=set()

        slen=0
        #(value_name, index, value, ipcon)
        self.value_items=[]
        #(struct_name, None/filed_name, value_name, index, value, ipcon)
        self.struct_items=[]
        self.struct_fnames={}
        self.header_addition=[]
        cline=""
        while True:
            nline=inf.readline()
            if nline=="": break
            nline=nline.rstrip()
            if self.proc_header_addition(nline): continue
            nline=nline.lstrip()
            if nline and len(nline)>0 and nline[-1]=='\\':
                cline+=nline[:-1]
                continue
            else:
                cline+=nline
            line=cline
            cline=""
            rc=recomment.match(line)
            if rc:
                line=rc.group(1)
                comment=rc.group(2)
            else:
                comment=''

            ipcon=0
            persistent=0
            line,sdq=SpaceInQuote.replace_spaces(line)
            for i, recomp in enumerate(recomps):
                r=recomp.match(line)
                if not r: continue

                if i==0:
                    # struct field names
                    self.struct_fnames[r.group(1)]=r.group(2).split(',')
                    break
                elif i==1:
                    # [IRWP], ipc or persistent
                    w=r.group(2)
                    if "I" in w:
                        if "R" in w: ipcon=1
                        if "W" in w: ipcon|=2
                        if "R" not in w and "W" not in w: ipcon=3
                    if "P" in w:
                        persistent=1
                    continue
                elif i==2:
                    # struct with index
                    self.struct_items.append((r.group(1), r.group(4),
                                              r.group(2), r.group(3),
                                              SpaceInQuote.back_to_spaces(r.group(5),sdq), ipcon, persistent))
                    break
                elif i==3:
                    # struct without index
                    self.struct_items.append((r.group(1), r.group(3),
                                              r.group(2), None,
                                              SpaceInQuote.back_to_spaces(r.group(4),sdq), ipcon, persistent))
                    break
                elif i==4:
                    # value with index
                    self.value_items.append((r.group(1), r.group(2),
                                             SpaceInQuote.back_to_spaces(r.group(3), sdq),
                                             ipcon, persistent))
                    break
                elif i==5:
                    # value without index
                    self.value_items.append((r.group(1), None,
                                             SpaceInQuote.back_to_spaces(r.group(2), sdq),
                                             ipcon, persistent))
                    break

            if self.toutf:
                self.tempconf_write(i, r, line, sdq, comment)

        inf.close()
        if self.toutf: self.toutf.close()

    def proc_header_addition(self, nline, noadd=False):
        if nline=="### HEADER ADDITION ###":
            self.in_header_addition=True
            return True
        if nline=="### END HEADER ADDITION ###":
            self.in_header_addition=False
            return True
        if not self.in_header_addition: return False
        if noadd: return True
        self.header_addition.append(nline)
        return True

    def file_preproc(self, fname):
        inf=open(fname)
        replace_words=[]
        self.persistent_fname=None
        while True:
            line=inf.readline()
            if line=='': break
            line=line.rstrip()
            if self.proc_header_addition(line, noadd=True): continue
            items=line.split()
            if len(items)==0: continue
            if items[0]=="UNIPAC_PERSISTENT":
                self.persistent_fname="(char*)%sconf_get_item(UNIPAC_PERSISTENT)" % self.pf
                continue
            if len(items)<3: continue
            if items[0]!="#define": continue
            replace_words.append((items[1],items[2]))
        inf.seek(0)
        outf=StringIO()
        in_addition=False
        while True:
            line=inf.readline()
            if line=="": break
            if line.find("### HEADER ADDITION ###")==0:
                in_addition=True
            if line.find("### END HEADER ADDITION ###")==0:
                in_addition=False
            if not in_addition:
                for rw in replace_words:
                    line=line.replace(rw[0], rw[1])
            outf.write(line)
        outf.seek(0)
        inf.close()
        return outf

    def tempconf_write(self, rei, reo, line, sdq, comment):
        if comment and comment.find("define ")!=0: self.toutf.write("#%s\n" % comment)
        if rei==1:
            self.toutf.write(line)
            return
        if not reo: return
        vname=''
        index=''
        findex=''
        value=''
        cols=False
        dots=''
        if rei==2:
            if reo.group(4):
                dots='.'
                findex=reo.group(4)
            vname=reo.group(2)
            index=reo.group(3)
            value=SpaceInQuote.back_to_spaces(reo.group(5),sdq)
            if index: cols=True
        elif rei==3:
            if reo.group(3):
                dots='.'
                findex=reo.group(3)
            vname=reo.group(2)
            value=SpaceInQuote.back_to_spaces(reo.group(4), sdq)
        elif rei==4:
            vname=reo.group(1)
            index=reo.group(2)
            value=SpaceInQuote.back_to_spaces(reo.group(3), sdq)
            if index: cols=True
        elif rei==5:
            vname=reo.group(1)
            value=SpaceInQuote.back_to_spaces(reo.group(2), sdq)
        if not vname: return

        value=re.sub(r'"\[[0-9]*\]','"', value)
        if cols:
            self.toutf.write("#%s[%s]%s%s %s\n" % (vname, index, dots, findex, value))
        else:
            self.toutf.write("#%s%s%s %s\n" % (vname, dots, findex, value))
        self.toutf.write("\n")

    def vname_update(self, vname, v, index=None, findex=None):
        if index==None:
            if findex==None:
                if self.ditems[vname]['values'].__class__==ValueAtom:
                    self.ditems[vname]['values']=v.get_first_atom()
                else:
                    self.ditems[vname]['values']=v
                logger.debug("update data:vname=%s, values=%s" % \
                             (vname, self.ditems[vname]['values']))
            else:
                if self.ditems[vname]['values'][findex].__class__==ValueAtom:
                    self.ditems[vname]['values'][findex]=v.get_first_atom()
                else:
                    self.ditems[vname]['values'][findex]=v
                logger.debug("update data:vname=%s, findex=%d, values=%s" % \
                             (vname, findex, self.ditems[vname]['values'][findex]))
        else:
            if findex==None:
                if self.ditems[vname]['values'][index].__class__==ValueAtom:
                    self.ditems[vname]['values'][index]=v.get_first_atom()
                else:
                    self.ditems[vname]['values'][index]=v
                logger.debug("update data:vname=%s, index=%d, values=%s" % \
                             (vname, index, self.ditems[vname]['values']))
            else:
                if self.ditems[vname]['values'][index][findex].__class__==ValueAtom:
                    self.ditems[vname]['values'][index][findex]=v.get_first_atom()
                else:
                    self.ditems[vname]['values'][index][findex]=v
                logger.debug("update data:vname=%s, index=%d, findex=%d, values=%s" % \
                             (vname, index, findex, self.ditems[vname]['values'][index][findex]))

    def proc_allitems(self):
        self.ditems={}
        for vi in self.value_items:
            self.proc_oneitem(vi)
        for vi in self.struct_items:
            fnames= self.struct_fnames[vi[0]] if vi[0] in self.struct_fnames else None
            self.proc_oneitem(vi[2:], struct=vi[0], fi=vi[1], fnames=fnames)

    def get_findex_fname(self, fi, fnames):
        for i,x in enumerate(fnames):
            if fi==x: return i
        return None

    def proc_oneitem(self, vi, struct=None, fi=None, fnames=None):
        vname=vi[0]
        if vname not in self.ditems:
            self.ditems[vname]={'struct':struct, 'ipcon':vi[3], 'persistent':vi[4], 'values':[], 'pi':-1}

        if struct:
            vs,n=find_brapair(vi[2], '{')
        else:
            vs=vi[2]

        try:
            v=ValueUnit(vs, struct=struct)
        except ValueError:
            logger.error("bad format of data:%s" % vi)
            return

        if not self.ditems[vname]['values']:
            # initial definitions
            self.ditems[vname]['values']=v
            logger.debug("new data:vname=%s, values=%s" % (vname, self.ditems[vname]['values']))
        else:
            # overwriting definitions
            nei=None
            if vi[1]==None:
                pass
            elif vi[1]=='':
                nei=self.ditems[vname]['pi']+1
            else:
                nei=int(vi[1])

            if nei and nei>=len(self.ditems[vname]['values']):
                logger.error("index:%d is out of range:%d" % \
                             (nei, len(self.ditems[vname]['values'])))
                return -1
            if not fi:
                self.vname_update(vname, v, index=nei)
            else:
                try:
                    findex=int(fi[1:])
                except ValueError:
                    if fnames: findex=self.get_findex_fname(fi, fnames)
                if findex==None:
                    logger.error("the field name:'%s' is wrong" % fi)
                    return -1
                self.vname_update(vname, v, index=nei, findex=findex)

            if nei!=None: self.ditems[vname]['pi']=nei

        if struct:
            if self.ditems[vname]['values'].array:
                self.struct_defs.register_struct(struct, self.ditems[vname]['values'][0], fnames)
            else:
                self.struct_defs.register_struct(struct, self.ditems[vname]['values'], fnames)

    def register_string_types(self, st):
        for s in st:
            sn=int(s[ValueAtom.STRTPREFLEN:])
            self.string_types_set|={sn}

    def scan_string_types(self):
        for vname, v in self.ditems.items():
            for v1 in v['values']:
                st=v1.get_string_types()
                if not st: continue
                self.register_string_types(st)

    def get_vtype_ens(self, v):
        res={'vtype':None, 'ens0':None, 'ens1':None}
        if len(v['values'])>=1 and v['values'].array:
            res['ens0']=len(v['values'])
        if v['struct']:
            res['vtype']=v['struct']+"_t"
        else:
            res['vtype']=v['values'].get_first_smax_vtype()
            if not res['ens0']: v['values'].str_nobraces=True
            if v['values'].max_len_in_children()>1:
                res['ens1']=v['values'].max_len_in_children()
        return res

    def set_thirdelement_type_set(self):
        for vname,v in self.ditems.items():
            vtens=self.get_vtype_ens(v)
            if vtens['ens0'] and vtens['ens1'] and \
               isinstance(v['values'][0][0], ValueUnit) and len(v['values'][0][0])>1:
                etype="%s_%dt" % (v['values'][0][0][0][0].upper(), len(v['values'][0][0]))
                self.thirdelement_type_set|={ \
                    (v['values'][0][0][0][0], etype, len(v['values'][0][0]))}

class ConfigOutput(object):
    def __init__(self, scanconfig, pf):
        super().__init__()
        self.scanconfig=scanconfig
        self.outf=StringIO()
        self.pf=pf

    def write_from_temp(self, fname, tempfname=None, bt=None):
        soutf=open(fname, "w")
        soutf.write("/* Don't edit this file.  This is an automatically generated file. */\n")
        if tempfname:
            inf=open(tempfname, "r")
            temptext=inf.read()
            inf.close()
            temptext=temptext.replace("_CONFPREFIX_", self.pf)
        else:
            temptext="/*_CONF_GENERATED_*/"
        self.outf.seek(0)
        generated_text=self.outf.read()
        temptext=temptext.replace("/*_CONF_GENERATED_*/", generated_text)
        ep=temptext.rfind("#endif")
        if ep>0:
            soutf.write(temptext[:ep])
            if bt:
                soutf.write(bt)
                soutf.write("\n")
            soutf.write(temptext[ep:])
        else:
            soutf.write(temptext)
            if bt:
                soutf.write(bt)
        soutf.close()

    def close(self):
        self.outf.close()

class ConfigHeaderOutput(ConfigOutput):
    def __init__(self, scanconfig, pf, packed=False):
        super().__init__(scanconfig, pf)
        self.packed=packed

    def print_string_types(self):
        for i in self.scanconfig.string_types_set:
            if i<16: continue
            self.outf.write("#ifndef UP_%s%d_DEFINED\n" % (ValueAtom.STRTPREF.upper(), i))
            self.outf.write("#define UP_%s%d_DEFINED\n" % (ValueAtom.STRTPREF.upper(), i))
            self.outf.write("typedef char %s%d[%d];\n" % (ValueAtom.STRTPREF, i, i+1))
            self.outf.write("#endif\n")
        self.outf.write("\n")

    def print_thirdelement_types(self):
        self.scanconfig.set_thirdelement_type_set()
        for i in self.scanconfig.thirdelement_type_set:
            self.outf.write("#ifndef %s_DEFINED\n" % i[1])
            self.outf.write("#define %s_DEFINED\n" % i[1])
            self.outf.write("typedef %s %s[%d];\n" % (i[0],i[1],i[2]))
            self.outf.write("#endif\n")
        self.outf.write("\n")

    def print_struct(self):
        nps="__attribute__((packed))" if self.packed else ""
        for key, values in self.scanconfig.struct_defs.items():
            self.outf.write("typedef struct %s {\n" % (key))
            for i, v in enumerate(values):
                av1='' if v[1]==1 else '[%d]' % v[1]
                av2='' if v[2]==1 else '[%d]' % v[2]
                self.outf.write("\t%s %s%s%s;\n" % (v[0], v[3], av1, av2))
            self.outf.write("}%s %s_t;\n" % (nps, key))
            self.outf.write("\n")

    def print_enum(self):
        self.outf.write("typedef enum {\n")
        self.outf.write("\t%sCONF_ENUM_NON_ITEM=-1,\n" % self.pf)
        for key in self.scanconfig.ditems.keys():
            self.outf.write("\t%s,\n" % key)
        self.outf.write("\t%sCONF_ENUM_LAST_ITEM\n" % self.pf)
        self.outf.write("} %s_config_item_t;\n" % self.pf)
        self.outf.write("\n")

        self.outf.write("#ifndef UP_VTYPE_DEFINED\n")
        self.outf.write("#define UP_VTYPE_DEFINED\n")
        self.outf.write("#define UPSTRING_TYPE_BASE %d\n" % (ValueTypes.STRING_TYPE_BASE))
        self.outf.write("typedef enum {\n")
        for k,v in sorted(value_types.items(), key=lambda x: x[1]):
            self.outf.write("\tVT_%s=%d,\n" % (k.upper(), v))
        self.outf.write("\tVT_ENUM_LAST_NUM\n")
        self.outf.write("} up_config_struct_t;\n")
        self.outf.write("\n")

        for i in range(16):
            self.outf.write("typedef char %s%d[%d];\n" % (ValueAtom.STRTPREF, i, i+1))

        ctext='''
#define UPIPC_N (0)
#define UPIPC_R (1)
#define UPIPC_W (1<<1)
#define UPIPC_RW ((1)|(1<<1))
'''
        self.outf.write(ctext)
        self.outf.write("#endif\n")
        self.outf.write("\n")

    def print_struct_enum(self):
        self.outf.write("typedef enum {\n")
        for key in self.scanconfig.struct_defs.keys():
            self.outf.write("\t%s,\n" % key)
        self.outf.write("\t%sCONF_ENUM_LAST_STRUCT\n" % self.pf)
        self.outf.write("} %s_config_struct_t;\n" % self.pf)
        self.outf.write("\n")

    def print_header_addition(self):
        self.scanconfig.header_addition.append('')
        return "\n".join(self.scanconfig.header_addition)

class ConfigCodeOutput(ConfigOutput):
    def print_config_item_strings(self):
        self.outf.write("static const char *config_item_strings[]={\n")
        for key in self.scanconfig.ditems.keys():
            self.outf.write("\t\"%s\",\n" % key)
        self.outf.write("};\n")
        self.outf.write("\n")

    def print_config_item_ipcon(self):
        self.outf.write("static uint8_t config_item_ipcon[]={\n")
        self.outf.write("\t")
        for i,v in enumerate(self.scanconfig.ditems.values()):
            if v['ipcon']==0: self.outf.write("UPIPC_N ,")
            elif v['ipcon']==1: self.outf.write("UPIPC_R ,")
            elif v['ipcon']==2: self.outf.write("UPIPC_W ,")
            elif v['ipcon']==3: self.outf.write("UPIPC_RW,")
            else: raise ValueError
            if i%5==4:
                self.outf.write("\n\t")
        self.outf.write("};\n")
        self.outf.write("\n")

    def print_config_item_persistent(self):
        self.outf.write("static const int8_t config_item_persistent[]={\n")
        self.outf.write("\t")
        for i,v in enumerate(self.scanconfig.ditems.keys()):
            if self.scanconfig.ditems[v]['persistent']:
                self.outf.write("1, ")
            else:
                self.outf.write("0, ")
            if i%16==15:
                self.outf.write("\n\t")
        self.outf.write("};\n")
        self.outf.write("\n")

    def get_persistent_rw_items(self, fobj='outf'):
        frwlist=[]
        vs=list(self.scanconfig.ditems.keys())
        vs.sort()
        if fobj=='outf':
            fcmd='fwrite'
        else:
            fcmd='fread'
        for v in vs:
            vi=self.scanconfig.ditems[v]
            if not vi['persistent']: continue
            vtens=self.scanconfig.get_vtype_ens(vi)
            en=0
            if vtens['ens0']: en=vtens['ens0']
            if vtens['ens1']: en=max(1, en)*vtens['ens1']
            if en:
                frwlist.append('\tif(%s(v_%s, sizeof(%s), %d, %s)!=%d) res=1;\n' % \
                               (fcmd, v, vtens['vtype'], en, fobj, en))
            else:
                frwlist.append('\tif(%s(&v_%s, sizeof(%s), 1, %s)!=1) res=1;\n' % \
                               (fcmd, v, vtens['vtype'], fobj))
        frwlist.append('\tif(res){\n')
        frwlist.append('\t\tUB_LOG(UBL_ERROR, "%s: wrong return value\\n", __func__);\n')
        frwlist.append('\t}\n')
        return frwlist

    def print_persistent_save_restore(self, ctext, fobj):
        self.outf.write("{\n")
        if not self.scanconfig.persistent_fname:
            self.outf.write("}\n")
            self.outf.write("\n")
            return
        self.outf.write("\tconst char *fname=%s;\n" % self.scanconfig.persistent_fname)
        self.outf.write(ctext)
        frwlist=self.get_persistent_rw_items(fobj)
        self.outf.write("".join(frwlist))
        self.outf.write("\tfclose(%s);\n" % fobj)
        self.outf.write("}\n")
        self.outf.write("\n")

    def print_persistent_save(self):
        self.outf.write("static void persistent_save(bool force)\n")
        ctext='''
	FILE *outf;
        int res=0;
	if(!fname || fname[0]==0) return;
	if(!force && !persistent_dirty) return;
	if(!force &&
	   ub_mt_gettime64()-persistent_dirty_lastts<PERSISTENT_SAVE_INTERVAL)
		return;
	persistent_dirty=false;
	outf=fopen(fname, "w+");
	if(!outf){
		UB_LOG(UBL_ERROR, "%s:can't open fname=%s\\n", __func__, fname);
		return;
	}
'''
        self.print_persistent_save_restore(ctext, 'outf')
        self.outf.write("void %spersistent_save(void)\n" % self.pf)
        self.outf.write("{\n")
        self.outf.write("\tpersistent_save(true);\n")
        self.outf.write("}\n")
        self.outf.write("\n")

    def print_persistent_restore(self):
        self.outf.write("static void persistent_restore(void)\n")
        ctext='''
	FILE *inf;
        int res=0;
	if(!fname || fname[0]==0) return;
	inf=fopen(fname, "r");
	if(!inf){
		UB_LOG(UBL_ERROR, "%s:can't open fname=%s\\n", __func__, fname);
		return;
	}
'''
        self.print_persistent_save_restore(ctext, 'inf')
        self.outf.write("void %spersistent_restore(void)\n" % self.pf)
        self.outf.write("{\n")
        self.outf.write("\tpersistent_restore();\n")
        self.outf.write("}\n")
        self.outf.write("\n")

    def print_config_struct_strings(self):
        self.outf.write("static char *config_struct_strings[]={\n")
        for key in self.scanconfig.struct_defs.keys():
            self.outf.write("\t\"%s\",\n" % key)
        self.outf.write("};\n")
        self.outf.write("\n")

        self.outf.write("typedef struct struct_field_names {\n")
        self.outf.write("\tint sitem;\n")
        self.outf.write("\tchar *nmstr;\n")
        self.outf.write("} struct_field_names_t;\n")
        self.outf.write("\n")

        self.outf.write("static struct_field_names_t config_struct_fnames[]={\n")
        for key in self.scanconfig.struct_defs.keys():
            if key not in self.scanconfig.struct_fnames: continue
            fsn=",".join(self.scanconfig.struct_fnames[key])
            self.outf.write("\t{%s, \"%s\"},\n" % (key, fsn))
        self.outf.write("\t{-1, NULL},\n")
        self.outf.write("};\n");
        self.outf.write("\n")

        self.outf.write("static struct_field_names_t struct_fnames_variables[]={\n")
        for vname,v in self.scanconfig.ditems.items():
            if not v['struct']: continue
            if v['struct'] not in self.scanconfig.struct_fnames: continue
            self.outf.write("\t{%s, \"%s\"},\n" % (v['struct'], vname))
        self.outf.write("\t{-1, NULL},\n")
        self.outf.write("};\n");
        self.outf.write("\n")

    def print_config_values(self):
        for vname,v in self.scanconfig.ditems.items():
            vtens=self.scanconfig.get_vtype_ens(v)
            ens=""
            if vtens['ens0']: ens+="[%d]" % vtens['ens0']
            if vtens['ens1']: ens+="[%d]" % vtens['ens1']
            if vtens['ens0'] and vtens['ens1'] and \
               isinstance(v['values'][0][0], ValueUnit) and len(v['values'][0][0])>1:
                etype="%s_%dt" % (v['values'][0][0][0][0].upper(), len(v['values'][0][0]))
                self.outf.write("static %s v_%s%s=%s;\n" % \
                                (etype, vname, ens, str(v['values'])))
            else:
                self.outf.write("static %s v_%s%s=%s;\n" % \
                                (vtens['vtype'], vname, ens, str(v['values'])))

    def print_pointers_list(self):
        self.outf.write("static void *config_value_pointers[]={\n")
        for key,v in self.scanconfig.ditems.items():
            vtype=v['values'].get_first_smax_vtype()
            if v['struct']:
                if v['values'].array:
                    self.outf.write("\tv_%s,\n" % key)
                else:
                    self.outf.write("\t&v_%s,\n" % key)
            else:
                if v['values'].number_of_tuples()>1:
                    self.outf.write("\tv_%s,\n" % key)
                else:
                    self.outf.write("\t&v_%s,\n" % key)
        self.outf.write("};\n")
        self.outf.write("\n")

        self.outf.write("typedef int config_vs_t[4];\n")
        self.outf.write("//{size_of_value, num_of_values, num_of_element, value_type\n")
        self.outf.write("//num_of_values: how many values in one element\n")
        self.outf.write("//num_of_elements: how many elements in the item\n")
        self.outf.write("//value_type: struct type>=0, -1:invalid, predefined value type<-1\n")
        self.outf.write("static config_vs_t config_value_sizes[]={\n")
        for key,v in self.scanconfig.ditems.items():
            vtype=v['values'].get_first_smax_vtype()
            if v['struct']:
                sizestr="sizeof(%s_t)"%v['struct']
                uen=1
                if v['values'].array:
                    en=len(v['values'])
                else:
                    en=1
                st=v['struct']
            else:
                st="%s" % StructDefinitions.get_vtype_numstr(vtype)
                sizestr = StructDefinitions.get_sizeof_str(vtype)
                uen=v['values'].max_len_in_children()
                en=len(v['values'])
            sizestr="{%s, %d, %d, %s}" % (sizestr, uen, en, st)
            self.outf.write("\t%s,\n" % sizestr)
        self.outf.write("};\n")
        self.outf.write("\n")

    def print_struct_field_vtype(self):
        self.outf.write("int %sstruct_field_vtype(int sindex, int findex, "
                        "int *vsize, int *elen, int *vnum, int *foffset)\n" % self.pf)
        self.outf.write("{\n")
        self.outf.write("\tswitch(sindex){\n")
        for struct,v in self.scanconfig.struct_defs.items():
            self.outf.write("\tcase %s:\n" % struct)
            self.outf.write("\t{\n")
            self.outf.write("\t\t%s_t a;\n" % struct)
            self.outf.write("\t\tswitch(findex){\n")
            for i,f in enumerate(v):
                self.outf.write("\t\tcase %d:\n" % i)
                self.outf.write("\t\t\t*vsize=%s;\n" % StructDefinitions.get_sizeof_str(f[0]))
                self.outf.write("\t\t\t*vnum=%d;\n" % f[2])
                self.outf.write("\t\t\t*elen=%d;\n" % f[1])
                self.outf.write("\t\t\t*foffset=(void*)&a.%s-(void*)&a;\n" % f[3])
                self.outf.write("\t\t\treturn %s;\n" % StructDefinitions.get_vtype_numstr(f[0]))
            self.outf.write("\t\t};\n")
            self.outf.write("\t\tbreak;\n")
            self.outf.write("\t}\n")
        self.outf.write("\tdefault:\n")
        self.outf.write("\t\tbreak;\n")
        self.outf.write("\t};\n")
        self.outf.write("\treturn VT_INVALID;\n")
        self.outf.write("}\n")
        self.outf.write("\n")

    def print_struct_field_update(self):
        self.outf.write("static int %sstruct_field_update(int vtype, int findex, "
                        "void *variable, void *values, int sizev)\n" % self.pf)
        self.outf.write("{\n")
        self.outf.write("\tswitch(vtype){\n")
        for struct,v in self.scanconfig.struct_defs.items():
            self.outf.write("\tcase %s:\n" % struct)
            self.outf.write("\t{\n")
            self.outf.write("\t\t%s_t *v=(%s_t *)variable;\n" % (struct, struct))
            self.outf.write("\t\tswitch(findex){\n")
            for i,f in enumerate(v):
                self.outf.write("\t\tcase %d:\n" % i)
                self.outf.write("\t\t\tmemcpy(&v->%s, values, UB_MIN((int)%s*%d,sizev));\n" %
                                (f[3], StructDefinitions.get_sizeof_str(f[0]),
                                 f[1]*f[2]))

                self.outf.write("\t\t\treturn 0;\n")
            self.outf.write("\t\t}\n")
            self.outf.write("\t\tbreak;\n")
            self.outf.write("\t}\n")
        self.outf.write("\tdefault:\n")
        self.outf.write("\t\tbreak;\n")
        self.outf.write("\t}\n")
        self.outf.write("\treturn -1;\n")
        self.outf.write("}\n")
        self.outf.write("\n")

    def get_value_typecode(self):
        ctext='''
	case VT__VTYPEUPPER_:
	{
		_VTYPE_ rv;
		for(i=0;i<elen && usize<*esize;i++){
			vp=v+i*vs*vnum;
			if(check_first_char(svalues, svalsize, '{')) ffb++;
			for(j=0;j<vnum && usize<*esize;j++){
				rn=get_bracket_num(svalues, svalsize, false);
				if(rn<0) rn=1;
				_SETRV_;

				vp = ((char*)vp) + vs*rn;
				usize+=vs*rn;
				*svalsize-=vn-*svalues;
				*svalues=vn;
				if(!check_first_char(svalues, svalsize, '_BCHAR_')) break;
				if(*svalsize<=0) break;
			}
			if(ffb && check_first_char(svalues, svalsize, '}')){
				ffb--;
				if(!check_first_char(svalues, svalsize, ',')) break;
			}
		}
		break;
	}
'''
        return ctext

    def print_get_value(self):
        ctext='''
static void *get_value(int vs, int elen, int vnum, int vtype,
		       char **svalues, int *svalsize, int *esize)
{
	void *v, *vp;
	char *vn;
	int ffb=0;
        int usize=0;
	int i,j;
	int rn,k;
	int res=0;
	if(*svalsize<=0) return NULL;
	v=malloc(vs*vnum*elen);
        memset(v,0,vs*vnum*elen);
	if(check_first_char(svalues, svalsize, '{')) ffb++;
	switch(vtype){
'''
        self.outf.write(ctext)
        for k,v in sorted(value_types.items(), key=lambda x: x[1]):
            if k=="invalid":
                continue;

            bc=","
            if k=="int8_t" or k=="int16_t" or k=="int32_t" or k=="int64_t" or \
               k=="uint32_t" or k=="uint64_t":
                us="rv=strtol(*svalues, &vn, 0);if(*svalues==vn){break;};for(k=0;k<rn;k++){memcpy(vp+vs*k, &rv, vs);}"
            elif k=="float":
                us="rv=strtof(*svalues, &vn);if(*svalues==vn){break;};for(k=0;k<rn;k++){memcpy(vp+vs*k, &rv, vs);}"
            elif k=="double":
                us="rv=strtod(*svalues, &vn);if(*svalues==vn){break;};for(k=0;k<rn;k++){memcpy(vp+vs*k, &rv, vs);}"
            elif k=="bool":
                us="rv=strstr(*svalues, \"true\")==*svalues;" \
                "if(!rv && !strstr(*svalues, \"false\")){break;}" \
                "vn=*svalues+(rv?4:5);for(k=0;k<rn;k++){memcpy(vp+vs*k, &rv, vs);}"
            elif k=="uint8_t" or k=="uint16_t":
                us="rv=strtol(*svalues, &vn, 16);if(*svalues==vn){break;};for(k=0;k<rn;k++){memcpy(vp+vs*k, &rv, vs);}"
                bc=':'
            elif k=="char":
                us="rv=(*svalues)[1];vn=*svalues+3;for(k=0;k<rn;k++){memcpy(vp+vs*k, &rv, vs);}"
            else:
                continue

            ctext=self.get_value_typecode()
            ctext=ctext.replace("_VTYPEUPPER_", k.upper())
            ctext=ctext.replace("_VTYPE_", k)
            ctext=ctext.replace("_SETRV_", us)
            ctext=ctext.replace("_BCHAR_", bc)
            self.outf.write(ctext)


        ctext='''
	default:
		if(vtype<UPSTRING_TYPE_BASE){
			free(v);
			return NULL;
		}
'''
        self.outf.write(ctext)
        ctext=self.get_value_typecode()
        ctext=ctext.replace("	case VT__VTYPEUPPER_:\n", "")
        ctext=ctext.replace("_VTYPE_", "char*")
        us='''bool esc=false;
				int n=0;
				if(*svalues[0]!='"') break;
				if(!strchr(*svalues+1,'"')) break;
				for(rv=*svalues+1;rv<=*svalues+1+vs;rv++){
					if(!esc && *rv=='"') break;
					if(*rv=='\\\\'){
						esc=true;
						continue;
					}
					esc=false;
					((char*)vp)[n++]=*rv;
				}
				if(*rv!='"') break;
				((char*)vp)[n]=0;
				vn=rv+1'''
        ctext=ctext.replace("_SETRV_", us)
        ctext=ctext.replace("_BCHAR_", ",")
        self.outf.write(ctext)
        self.outf.write("	}\n")

        ctext='''
	if(ffb>1) res=-1;
	if(ffb==1 && !check_first_char(svalues, svalsize, '}')) res=-1;
	if(res){
		free(v);
		return NULL;
	}
	*esize=usize;
	return v;
}
'''
        self.outf.write(ctext)

class ConfigTestcodeOutput(ConfigOutput):
    def print_file_head(self, hfname):
        self.outf.write('#include <string.h>\n')
        self.outf.write('#include <stdio.h>\n')
        self.outf.write('#include "%s"\n' % hfname[hfname.rfind('/')+1:])
        self.outf.write('#include <xl4unibase/unibase.h>\n')
        self.outf.write('#include <xl4unibase/unibase_binding.h>\n')

    def print_numtest_function(self, vt):
        self.outf.write("static int %stest(void)\n" % (vt))
        self.outf.write("{\n")
        self.outf.write("\t%s v1,v2,v3;\n" % vt)
        for key,v in self.scanconfig.ditems.items():
            if v['struct']: continue
            if v['values'].get_first_vtype() != vt: continue
            if len(v['values'])>1: continue
            if vt=="float":
                fs="%f"
            else:
                fs='%"PRIi64"' if vt=="int64_t" else '%"PRIi32"'
            ls='l' if vt=="int64_t" else ''
            if vt=='float':
                self.outf.write("\tv1=*(%s*)%sconf_get_item(%s);\n" % (vt, self.pf, key))
            else:
                self.outf.write("\tv1=%sconf_get_%sintitem(%s);\n" % (self.pf, ls, key))
            self.outf.write("\tv2=v1*2;\n")
            self.outf.write("\t%sconf_set_stritem(\"%s\", &v2);\n" % (self.pf, key))
            if vt=='float':
                self.outf.write("\tv3=*(%s*)%sconf_get_item(%s);\n" % (vt, self.pf, key))
            else:
                self.outf.write("\tv3=%sconf_get_%sintitem(%s);\n" % (self.pf, ls, key))
            self.outf.write("\tif(v2!=v3) {printf(\"%s v2=%s,v3=%s\\n\", v2, v3); return -1;}\n" % \
                            (key, fs, fs))
            self.outf.write("\tif(v1!=v3/2) {printf"
                            "(\"%s v1=%s,v3=%s\\n\", v1, v3);}\n" % (key, fs, fs))
        self.outf.write("\treturn 0;\n")
        self.outf.write("}\n")

    def print_chartest_function(self):
        self.outf.write("static int chartest(void)\n")
        self.outf.write("{\n")
        self.outf.write("\tchar *p1,*p2;\n")
        for key,v in self.scanconfig.ditems.items():
            if v['struct']: continue
            if v['values'].get_first_vtype() != 'char': continue
            if v['values'].max_len_in_children()>1: continue
            self.outf.write("\tp1=\"%s\";\n" % ('z'*(len(v['values'])-1)))
            self.outf.write("\t%sconf_set_item(%s, p1);\n" % (self.pf, key))
            self.outf.write("\tp2=%sconf_get_item(%s);\n" % (self.pf, key))
            self.outf.write("\tif(strcmp(p1,p2)) "
                            "{printf(\"%s p1=%%s,p2=%%s\\n\", p1, p2); return -1;}\n" % key)
        self.outf.write("\treturn 0;\n")
        self.outf.write("}\n")

    def print_numarraytest_function(self, vt):
        self.outf.write("static int %sarraytest(void)\n" % (vt))
        self.outf.write("{\n")
        self.outf.write("\tint i,j;\n")
        self.outf.write("\t%s v1[10],v2[10],v3;\n" % vt)
        for key,v in self.scanconfig.ditems.items():
            if v['struct']: continue
            if v['values'].get_first_vtype() != vt: continue
            if len(v['values'])<=1: continue
            if vt=="float":
                fs="%f"
            else:
                fs='%"PRIi64"' if vt=="int64_t" else '%"PRIi32"'
            ls='l' if vt=="int64_t" else ''
            self.outf.write("\tfor(i=0;i<%d;i++){\n" % len(v['values']))
            self.outf.write("\t\tv1[i]=*(%s*)%sconf_get_item_index(%s,i);\n" % (vt, self.pf, key))
            self.outf.write("\t};\n")
            self.outf.write("\tv3=1000;\n")
            self.outf.write("\tfor(i=0;i<%d;i++){\n" % len(v['values']))
            self.outf.write("\t\tv3--;\n")
            self.outf.write("\t\t%sconf_set_item_index(%s,&v3, i);\n" % (self.pf, key))
            self.outf.write("\t\tfor(j=0;j<%d;j++) v2[j]=*(%s*)%sconf_get_item_index(%s,j);\n" %
                            (len(v['values']), vt, self.pf, key))
            self.outf.write("\t\tv1[i]=v3;\n")
            self.outf.write("\t\tif(memcmp(v1, v2, %sconf_get_item_element_size(%s))) {\n" % \
                            (self.pf ,key))
            self.outf.write("\t\t\tprintf(\"%s failed\\n\"); return -1;\n" % key)
            self.outf.write("\t\t};\n")
            self.outf.write("\t};\n")
            self.outf.write("\n")
        self.outf.write("\treturn 0;\n")
        self.outf.write("}\n")

    def print_structtest_function(self):
        self.outf.write("static int structtest(void)\n")
        self.outf.write("{\n")
        self.outf.write("\tuint8_t vp[30][1024];\n")
        self.outf.write("\tvoid *v;\n")
        self.outf.write("\tint i;\n")
        for key,v in self.scanconfig.ditems.items():
            if not v['struct']: continue
            if len(v['values'])<=3: continue
            self.outf.write("\t\n")
            self.outf.write("\tfor(i=0;i<%sconf_get_item_element_num(%s);i++){\n" % (self.pf, key))
            self.outf.write("\t\tmemcpy(vp[i],%sconf_get_item_index(%s,i),\n" % (self.pf, key))
            self.outf.write("\t\t\t%sconf_get_item_element_size(%s));\n" % (self.pf, key))
            self.outf.write("\t};\n")
            self.outf.write("\t((%s_t*)vp[i-1])->%s=11111;\n" %
                            (v['struct'],self.scanconfig.struct_defs[v['struct']][0][3]))
            self.outf.write("\t%sconf_set_item_index(%s, vp[i-1], i-1);\n" % (self.pf, key))
            self.outf.write("\tif(memcmp(vp[0],%sconf_get_item_index(%s,0),\n" % (self.pf, key))
            self.outf.write("\t\t\t%sconf_get_item_element_size(%s))) return -1;\n" %
                            (self.pf, key))
            self.outf.write("\tif(memcmp(vp[i-1],%sconf_get_item_index(%s,i-1),\n" % (self.pf, key))
            self.outf.write("\t\t\t%sconf_get_item_element_size(%s))) return -1;\n" %
                            (self.pf, key))
            self.outf.write("\tv=%sconf_get_item_index(%s,i-1);\n" % (self.pf, key))
            self.outf.write("\tif(((%s_t*)v)->%s!=11111) return -1;\n" %
                            (v['struct'],self.scanconfig.struct_defs[v['struct']][0][3]))
            self.outf.write("\n")
        self.outf.write("\treturn 0;\n")
        self.outf.write("}\n")

    def print_main_function(self):
        self.print_numtest_function('int32_t')
        self.print_numtest_function('int64_t')
        self.print_numtest_function('float')
        self.print_chartest_function()
        self.print_numarraytest_function('int32_t')
        self.print_structtest_function()
        self.outf.write("\n")
        self.outf.write("int main(int argc, char* argv[])\n")
        self.outf.write("{\n")
        self.outf.write("\tunibase_init_para_t init_para;\n")
        self.outf.write("\tubb_default_initpara(&init_para);\n")
        self.outf.write('\tunibase_init(&init_para);\n')
        self.outf.write("\tif(int32_ttest()) return -1;\n")
        self.outf.write("\tif(int64_ttest()) return -1;\n")
        self.outf.write("\tif(floattest()) return -1;\n")
        self.outf.write("\tif(chartest()) return -1;\n")
        self.outf.write("\tif(int32_tarraytest()) return -1;\n")
        self.outf.write("\tif(structtest()) return -1;\n")
        self.outf.write("\tunibase_close();\n")
        self.outf.write("\treturn 0;\n")
        self.outf.write("}\n")

class ConfigIpcHeaderOutput(ConfigOutput):
    pass

class ConfigIpcCodeOutput(ConfigOutput):
    def print_single_value_to_string(self):
        ctext='''
static int single_value_to_string(int vtype, char **vstr, int *vstrsize, int *vpi,
				  void **values, int *valsize)
{
	int res=0;
	if(*vstrsize-*vpi<16){
		if(realloc_vstr(vstr, vstrsize)) return -1;
	}
	switch(vtype){
	case VT_BOOL:
		if(*valsize<(int)sizeof(bool)) return -1;
		if(*((bool *)*values))
			append_string(vstr, vstrsize, vpi, "true");
		else
			append_string(vstr, vstrsize, vpi, "false");
		*values = ((char*)(*values)) + sizeof(bool);
		*valsize-=sizeof(bool);
		break;
'''
        self.outf.write(ctext)
        for k,v in sorted(value_types.items(), key=lambda x: x[1]):
            if k=="invalid" or k=="bool":
                continue;
            elif k=="float" or k=="double":
                ps="\"%f\""
            elif k=="char":
                ps="\"'%c'\""
            else:
                if k.find('uint')==0:
                    n=k.find('_')
                    bn=int(k[4:n])
                    ps="\"%%\"PRIu%d" % bn
                elif k.find('int')==0:
                    n=k.find('_')
                    bn=int(k[3:n])
                    ps="\"%%\"PRIi%d" % bn
                else:
                    continue

            ctext='''
	case VT__VTYPEUPPER_:
		if(*valsize<(int)sizeof(_VTYPE_)) return -1;
		res=snprintf(*vstr+*vpi, *vstrsize, _VTYPEPRI_, *((_VTYPE_*)*values));
		if(res<0) return -1;
		*vpi+=res;
		*valsize-=sizeof(_VTYPE_);
		*values = ((char*)(*values)) + sizeof(_VTYPE_);
		break;
'''
            ctext=ctext.replace("_VTYPEUPPER_", k.upper())
            ctext=ctext.replace("_VTYPE_", k)
            ctext=ctext.replace("_VTYPEPRI_", ps)
            self.outf.write(ctext)

        self.outf.write("\tdefault:\n")
        ctext='''
	{
		int n;
		if(vtype<UPSTRING_TYPE_BASE) return -1;
		n=vtype-UPSTRING_TYPE_BASE+1;
		if(*valsize<n) return -1;
		if(*vstrsize-*vpi<n+2){
			if(realloc_vstr(vstr, vstrsize)) return -1;
		}
		res=snprintf(*vstr+*vpi, UB_MIN(*vstrsize, n+2), "\\"%s\\"", (char*)*values);
		*vpi+=res;
		*valsize-=n;
		*values = ((char*)(*values)) + n;
		break;
	}
'''
        self.outf.write(ctext)

        ctext='''
	}
	if(vtype==VT_CHAR && *(*vstr+*vpi-2)==0){
		*(*vstr+*vpi-2)='\\\\';
		*(*vstr+*vpi-1)='0';
		*(*vstr+*vpi)='\\'';
		*(*vstr+*vpi+1)=0;
		*vpi+=1;
	}
	return 0;
}
'''
        self.outf.write(ctext)

def print_usage():
    pname=sys.argv[0][sys.argv[0].rfind('/')+1:]
    print("%s [options]" % pname)
    print("    -h|--help: this help")
    print("    -i|--input: input file name")
    print("    -p|--prefix: prefix")
    print("    -v|--verbose level [3:debug|2:info|1:warn|0:error] (default=2)")
    print("    -c|--cfile: output c source file")
    print("    -d|--hfile: output header file")
    print("    -l|--clfile: output client appli source file")
    print("    -e|--exth: extra header files(comma separated list)")
    print("    -t|--tfile: output test c source file")
    print("    -m|--tmpcfg: output a template config file")
    print("    -n|--ipcfile: output IPC file")
    print("    -q|--ntfile: output non-thread header file")
    print("    -a|--tmpdir: template files directory")
    print("    -b|--srvfile: output a sample IPC server file")
    print("    -k|--packed: use packed struct")

def set_options():
    try:
        opts, args = getopt.getopt(sys.argv[1:], "hi:v:cde:p:tm:nlqa:bk",
                                   ["help", "input=", "verbose=", "cfile", "hfile",
                                    "exth=", "prefix=", "tfile", "tmpcfg=", "ipcfile",
                                    "clfile", "ntfile", "tmpdir=", 'srvfile',
                                    'packed'])
    except getopt.GetoptError as err:
        # print help information and exit:
        print(str(err))  # will print something like "option -a not recognized"
        print_usage()
        sys.exit(1)
    res={'input':'sample_defaults.cfg', 'verbose':logging.INFO, 'hfile':False,
         'exth':None, 'prefix':'sample', 'cfile':False, 'tfile':False , 'tmpcfg':None,
         'ipcfile':False, 'clfile':False, 'ntfile':False, 'tmpdir':'.', 'srvfile':False,
         'packed':False}
    for o, a in opts:
        if o in ("-h", "--help"):
            print_usage()
            sys.exit(0)
        elif o in ("-i", "--input"):
            res['input'] = a
        elif o in ("-v", "--verbose"):
            if a=='0' or a=='error': v=logging.ERROR
            elif a=='1' or a=='warn': v=logging.WARNING
            elif a=='2' or a=='info': v=logging.INFO
            elif a=='3' or a=='debug': v=logging.DEBUG
            else: v=logging.INFO
            res['verbose'] = v
        elif o in ("-d", "--hfile"):
            res['hfile'] = True
        elif o in ("-e", "--exth"):
            res['exth'] = a
        elif o in ("-c", "--cfile"):
            res['cfile'] = True
        elif o in ("-l", "--clfile"):
            res['clfile'] = True
        elif o in ("-p", "--prefix"):
            res['prefix'] = a
        elif o in ("-t", "--tfile"):
            res['tfile'] = True
        elif o in ("-m", "--tmpcfg"):
            res['tmpcfg'] = a
        elif o in ("-n", "--ipcfile"):
            res['ipcfile'] = True
        elif o in ("-q", "--ntfile"):
            res['ntfile'] = True
        elif o in ("-a", "--tmpdir"):
            res['tmpdir'] = a
        elif o in ("-b", "--srvfile"):
            res['srvfile'] = True
        elif o in ("-k", "--packed"):
            res['packed'] = True
        else:
            assert False, "unhandled option"
    return res

if __name__ == "__main__":
    options=set_options()
    logger.setLevel(options['verbose'])
    if os.access("%s/%s"% (options['tmpdir'],'temp_configs.h'), os.R_OK):
        tempdir=options['tmpdir']
    elif os.access("%s/%s"% ('/usr/local/share/xl4unipac','temp_configs.h'), os.R_OK):
        tempdir='/usr/local/share/xl4unipac'
    elif os.access("%s/%s"% ('/usr/share/xl4unipac','temp_configs.h'), os.R_OK):
        tempdir='/usr/share/xl4unipac'
    else:
        logger.error("can't find template files directory")
        sys.exit(1)

    pf=options['prefix']
    dc=ScanConfigFile(fname=options['input'], tfname=options['tmpcfg'], pf=options['prefix'])
    dc.proc_allitems()
    dc.scan_string_types()
    hfile="%s_configs.h" % pf
    cfile="%s_configs.c" % pf
    clfile="%s_ipcclient.c" % pf
    ipchfile="%s_ipcconfigs.h" % pf
    ipccfile="%s_ipcconfigs.c" % pf
    tfile="%s_configs_test.c" % pf
    ntfile="%s_non_thread.h" % pf
    srvfile="%s_ipcserver.c" % pf
    if options['hfile']:
        hc=ConfigHeaderOutput(dc, pf, packed=options['packed'])
        hc.print_enum()
        hc.print_string_types()
        hc.print_thirdelement_types()
        hc.print_struct_enum()
        hc.print_struct()
        bt=hc.print_header_addition()
        hc.write_from_temp(hfile, "%s/%s" % (tempdir, "temp_configs.h"), bt=bt)
        hc.close()

    if options['cfile']:
        cc=ConfigCodeOutput(dc, pf)
        cc.print_config_item_strings()
        cc.print_config_item_ipcon()
        cc.print_config_item_persistent()
        cc.print_config_struct_strings()
        cc.print_config_values()
        cc.print_pointers_list()
        cc.print_persistent_save()
        cc.print_persistent_restore()
        cc.print_struct_field_vtype()
        cc.print_struct_field_update()
        cc.print_get_value()
        cc.write_from_temp(cfile, "%s/%s" % (tempdir, "temp_configs.c"))
        cc.close()

    if options['tfile']:
        tc=ConfigTestcodeOutput(dc, pf)
        tc.print_file_head(hfile)
        tc.print_main_function()
        tc.write_from_temp(tfile)
        tc.close()

    if options['ipcfile']:
        ihc=ConfigIpcHeaderOutput(dc, pf)
        ihc.write_from_temp(ipchfile, "%s/%s" % (tempdir, "temp_ipcconfigs.h"))
        ihc.close()
        icc=ConfigIpcCodeOutput(dc, pf)
        icc.print_single_value_to_string()
        icc.write_from_temp(ipccfile, "%s/%s" % (tempdir, "temp_ipcconfigs.c"))
        icc.close()

    if options['clfile']:
        cl=ConfigOutput(dc, pf)
        cl.write_from_temp(clfile, "%s/%s" % (tempdir, "temp_ipcclient.c"))
        cl.close()

    if options['ntfile']:
        nt=ConfigOutput(dc, pf)
        nt.write_from_temp(ntfile, "%s/%s" % (tempdir, "temp_non_thread.h"))
        nt.close()

    if options['srvfile']:
        sv=ConfigOutput(dc, pf)
        sv.write_from_temp(srvfile, "%s/%s" % (tempdir, "temp_ipcserver.c"))
        sv.close()

    sys.exit(0)
