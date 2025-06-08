/* ----------------------------------------------------------------- */
/*           The Japanese TTS System "Open JTalk"                    */
/*           developed by HTS Working Group                          */
/*           http://open-jtalk.sourceforge.net/                      */
/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2008-2018  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the HTS working group nor the names of its  */
/*   contributors may be used to endorse or promote products derived */
/*   from this software without specific prior written permission.   */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <vector>
#include <emscripten/bind.h>

/* Main headers */
#include "mecab.h"
#include "njd.h"
#include "jpcommon.h"
#include "open_jtalk.h"

/* Sub headers */
#include "text2mecab.h"
#include "mecab2njd.h"
#include "njd_set_pronunciation.h"
#include "njd_set_digit.h"
#include "njd_set_accent_phrase.h"
#include "njd_set_accent_type.h"
#include "njd_set_unvoiced_vowel.h"
#include "njd_set_long_vowel.h"
#include "njd2jpcommon.h"

#define MAXBUFLEN 1024

typedef struct _Open_JTalk
{
   Mecab mecab;
   NJD njd;
   JPCommon jpcommon;
} Open_JTalk;

static void Open_JTalk_initialize(Open_JTalk *open_jtalk)
{
   Mecab_initialize(&open_jtalk->mecab);
   NJD_initialize(&open_jtalk->njd);
   JPCommon_initialize(&open_jtalk->jpcommon);
}

static void Open_JTalk_clear(Open_JTalk *open_jtalk)
{
   Mecab_clear(&open_jtalk->mecab);
   NJD_clear(&open_jtalk->njd);
   JPCommon_clear(&open_jtalk->jpcommon);
}

static int Open_JTalk_load(Open_JTalk *open_jtalk, char *dn_mecab)
{
   if (Mecab_load(&open_jtalk->mecab, dn_mecab) != TRUE)
   {
      Open_JTalk_clear(open_jtalk);
      return 0;
   }
   return 1;
}

static std::vector<Label> njd2feature(NJD *njd);

static std::vector<Label> Open_JTalk_run(Open_JTalk *open_jtalk, const char *txt)
{
   char buff[MAXBUFLEN];

   text2mecab(buff, txt);
   Mecab_analysis(&open_jtalk->mecab, buff);
   mecab2njd(&open_jtalk->njd, Mecab_get_feature(&open_jtalk->mecab),
             Mecab_get_size(&open_jtalk->mecab));
   njd_set_pronunciation(&open_jtalk->njd);
   njd_set_digit(&open_jtalk->njd);
   njd_set_accent_phrase(&open_jtalk->njd);
   njd_set_accent_type(&open_jtalk->njd);
   njd_set_unvoiced_vowel(&open_jtalk->njd);
   njd_set_long_vowel(&open_jtalk->njd);

   std::vector<Label> result = njd2feature(&open_jtalk->njd);

   NJD_refresh(&open_jtalk->njd);
   Mecab_refresh(&open_jtalk->mecab);

   return result;
}

static std::vector<Label> Open_JTalk_emscripten_run(const std::string &txt, const std::string &dictionary)
{
   Open_JTalk open_jtalk;
   Open_JTalk_initialize(&open_jtalk);

   if (!Open_JTalk_load(&open_jtalk, (char *)dictionary.c_str()))
   {
      Open_JTalk_clear(&open_jtalk);
      std::__throw_runtime_error("Error: Dictionary cannot be loaded.");
   }

   std::vector<Label> result = Open_JTalk_run(&open_jtalk, txt.c_str());

   Open_JTalk_clear(&open_jtalk);
   return result;
}

EMSCRIPTEN_BINDINGS(my_module)
{
   emscripten::value_object<Label>("Label")
       .field("string", &Label::string)
       .field("pos", &Label::pos)
       .field("pos_group1", &Label::pos_group1)
       .field("pos_group2", &Label::pos_group2)
       .field("pos_group3", &Label::pos_group3)
       .field("ctype", &Label::ctype)
       .field("cform", &Label::cform)
       .field("orig", &Label::orig)
       .field("read", &Label::read)
       .field("pron", &Label::pron)
       .field("acc", &Label::acc)
       .field("mora_size", &Label::mora_size)
       .field("chain_rule", &Label::chain_rule)
       .field("chain_flag", &Label::chain_flag);

   emscripten::register_vector<Label>("VectorLabel");

   emscripten::function("Open_JTalk", &Open_JTalk_emscripten_run);
}

Label node2feature(NJDNode *node);

static std::vector<Label> njd2feature(NJD *njd)
{
   std::vector<Label> labels;
   NJDNode *node = njd->head;
   while (node != NULL)
   {
      labels.push_back(node2feature(node));
      node = node->next;
   }

   return labels;
}

Label node2feature(NJDNode *node)
{
   Label feature;
   feature.string = (char *)NJDNode_get_string(node);
   feature.pos = (char *)NJDNode_get_pos(node);
   feature.pos_group1 = (char *)NJDNode_get_pos_group1(node);
   feature.pos_group2 = (char *)NJDNode_get_pos_group2(node);
   feature.pos_group3 = (char *)NJDNode_get_pos_group3(node);
   feature.ctype = (char *)NJDNode_get_ctype(node);
   feature.cform = (char *)NJDNode_get_cform(node);
   feature.orig = (char *)NJDNode_get_orig(node);
   feature.read = (char *)NJDNode_get_read(node);
   feature.pron = (char *)NJDNode_get_pron(node);
   feature.acc = NJDNode_get_acc(node);
   feature.mora_size = NJDNode_get_mora_size(node);
   feature.chain_rule = (char *)NJDNode_get_chain_rule(node);
   feature.chain_flag = NJDNode_get_chain_flag(node);
   return feature;
}

static void usage()
{
   fprintf(stderr, "The Japanese TTS System \"Open JTalk\"\n");
   fprintf(stderr, "Version 1.10 (http://open-jtalk.sourceforge.net/)\n");
   fprintf(stderr, "Copyright (C) 2008-2016 Nagoya Institute of Technology\n");
   fprintf(stderr, "All rights reserved.\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "Yet Another Part-of-Speech and Morphological Analyzer \"Mecab\"\n");
   fprintf(stderr, "Version 0.996 (http://mecab.sourceforge.net/)\n");
   fprintf(stderr, "Copyright (C) 2001-2008 Taku Kudo\n");
   fprintf(stderr, "              2004-2008 Nippon Telegraph and Telephone Corporation\n");
   fprintf(stderr, "All rights reserved.\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "NAIST Japanese Dictionary\n");
   fprintf(stderr, "Version 0.6.1-20090630 (http://naist-jdic.sourceforge.jp/)\n");
   fprintf(stderr, "Copyright (C) 2009 Nara Institute of Science and Technology\n");
   fprintf(stderr, "All rights reserved.\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "UniDic\n");
   fprintf(stderr, "Version 2.2.0 (https://unidic.ninjal.ac.jp/)\n");
   fprintf(stderr, "Copyright (C) 2011-2017 The UniDic Consortium\n");
   fprintf(stderr, "All rights reserved.\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "open_jtalk - The Japanese TTS system \"Open JTalk\"\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "  usage:\n");
   fprintf(stderr, "       open_jtalk [ options ] [ infile ] \n");
   fprintf(stderr,
           "  options:                                                                   [  def][ min-- max]\n");
   fprintf(stderr,
           "    -x  dir        : dictionary directory                                    [  N/A]\n");
   fprintf(stderr,
           "    -oo s          : filename of output wav audio (generated speech)         [  N/A]\n"); // TODO
   fprintf(stderr,
           "    -ot s          : filename of output trace information                    [  N/A]\n");
   fprintf(stderr, "  infile:\n");
   fprintf(stderr,
           "    text file                                                                [stdin]\n");
   fprintf(stderr, "\n");

   exit(0);
}

int main(int argc, char **argv)
{
   size_t i;

   /* text */
   char buff[MAXBUFLEN];

   /* Open JTalk */
   Open_JTalk open_jtalk;

   /* dictionary directory */
   char *dn_dict = NULL;

   /* input text file name */
   FILE *txtfp = stdin;
   char *txtfn = NULL;

   /* output file pointers */
   FILE *outfp = NULL;
   FILE *logfp = NULL;

   /* output usage */
   if (argc == 1)
      usage();

   /* initialize Open JTalk */
   Open_JTalk_initialize(&open_jtalk);

   /* get dictionary directory */
   for (i = 0; i < argc; i++)
   {
      if (argv[i][0] == '-' && argv[i][1] == 'x')
         dn_dict = argv[++i];
      if (argv[i][0] == '-' && argv[i][1] == 'h')
         usage();
   }
   if (dn_dict == NULL)
   {
      fprintf(stderr, "Error: Dictionary must be specified.\n");
      exit(1);
   }

   /* load dictionary */
   if (Open_JTalk_load(&open_jtalk, dn_dict) != TRUE)
   {
      fprintf(stderr, "Error: Dictionary cannot be loaded.\n");
      Open_JTalk_clear(&open_jtalk);
      exit(1);
   }

   /* get options */
   while (--argc)
   {
      if (**++argv == '-')
      {
         switch (*(*argv + 1))
         {
         case 'o':
            switch (*(*argv + 2))
            {
            case 'o':
               outfp = fopen(*++argv, "wb");
               break;
            case 't':
               logfp = fopen(*++argv, "wt");
               break;
            default:
               fprintf(stderr, "Error: Invalid option '-o%c'.\n", *(*argv + 2));
               exit(1);
            }
            --argc;
            break;
         case 'h':
            usage();
            break;
         case 'x':
            argv++; /* dictionary was already loaded */
            --argc;
            break;
         default:
            fprintf(stderr, "Error: Invalid option '-%c'.\n", *(*argv + 1));
            exit(1);
         }
      }
      else
      {
         txtfn = *argv;
         txtfp = fopen(txtfn, "rt");
      }
   }

   /* synthesize */
   fgets(buff, MAXBUFLEN - 1, txtfp);
   std::vector<Label> result = Open_JTalk_run(&open_jtalk, buff);
   if (result.empty())
   {
      fprintf(stderr, "Error: waveform cannot be synthesized.\n");
      Open_JTalk_clear(&open_jtalk);
      exit(1);
   }

   /* free memory */
   Open_JTalk_clear(&open_jtalk);

   /* close files */
   if (txtfn != NULL)
      fclose(txtfp);
   if (logfp != NULL)
      fclose(logfp);

   return 0;
}
