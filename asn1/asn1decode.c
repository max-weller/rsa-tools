
// Copyright (c) 2015 Max Weller

#include <stdio.h>

// source of information
// http://luca.ntop.org/Teaching/Appunti/asn1.html

char* tag_names[31] = {
NULL, NULL, 
"INTEGER", // 	2 	02
"BIT STRING", // 	3 	03
"OCTET STRING", // 	4 	04
"NULL", // 	5 	05
"OBJECT IDENTIFIER", // 	6 	06
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
"SEQUENCE and SEQUENCE OF", // 	16 	10
"SET and SET OF", // 	17 	11
NULL,
"PrintableString", // 	19 	13
"T61String", // 	20 	14
NULL,
"IA5String", // 	22 	16
"UTCTime", // 	23 	17
 NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

int len_dbg = 0;
int readpos = 0;

int print_indent(int indent) {
  while(indent --> 0) putchar('\t');
}

int parse_packed_int(FILE* f) {
  int c, out = 0, base = 0;
  while(!feof(f)) {
    c = fgetc(f);  readpos++;
    out += (c & 0x7F) << base;
    base += 7;
    if ((c & 0x80) == 0) return out;
  }
  return -1;
}
int parse_length_int(FILE* f) {
  int c = fgetc(f); readpos++;   if(len_dbg)printf("parselen %02X   ", c);
  if (c & 0x80) {
    int len = c & 0x7F;  if(len_dbg) printf(" long form %d ", len);
    if (len == 0) return -2;
    int out = 0, base = 0;
    while(len --> 0) {
      c = fgetc(f);  readpos++;   if(len_dbg)printf(" %02x ", c);
      out = (out << 8) + c;
    }
    return out;
  } else {
    if (len_dbg)printf ("  short form %d\n", c);
    return c;
  }
}
int hexdump(FILE* f, int len, int level) {
  int i = 0;
  print_indent(level);
  while (!feof(f) && i < len) {
    int c = fgetc(f);  readpos++;
    printf("%02X ", c);
    if (!(++i % 16)) { printf("\n"); print_indent(level); }
  }
  printf("\n");
}

#define MODE_EOF 0
#define MODE_FIXED 1
#define MODE_INDEF 2

int parse_der(FILE* f, int mode, int maxlen, int indent) {
  //print_indent(indent); printf("starting parse_der with mode=%d maxlen=%d indent=%d\n", mode,maxlen,indent);
  int c, i = 0;
  while(!feof(f)) {
    if (mode == MODE_FIXED && maxlen <= 0) break;
    readpos = 0;
    
    int tag, Class;
    c = fgetc(f);  readpos++;
    Class = (c & 0xC0) >> 6;
    if ((c & 0x1F) == 0x1F) {
      tag = parse_packed_int(f);
    } else {
      tag = c & 0x1F;
    }
    int cons = c & 0x20;
    
    int len = parse_length_int(f); 
    
    
    print_indent(indent);
    char* typeName = NULL;
    if (tag < 31) typeName = tag_names[tag];
    printf("Item Class %d, Tag %d (%s), Length %d, Constructed %d \n", Class, tag, typeName ? typeName : "unknown", len, !!cons);
    if (cons || tag == 4) {
      parse_der(f, len == -2 ? MODE_INDEF : MODE_FIXED, len, indent + 1);
    } else {
      hexdump(f, len, indent);
    }
    
     
    maxlen -= readpos;
  }
  
  
}

int main(int argc, char** argv) {
  
  FILE* f = fopen(argv[1], "r");
  if (!f) { perror("fopen"); return 1; }
  
  parse_der(f, 0, 0, 0);
  
  
}

