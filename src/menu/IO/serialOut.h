/* -*- C++ -*- */
#pragma once
////////////////////////////////////////////////////
// Rui Azevedo - Apr2019
// neu-rah (ruihfazevedo@gmail.com)
// use arduino Serial as menu output

#include <Arduino.h>
#include <streamFlow.h>
#include "../out.h"
#include "../nav.h"
#include "../fmt/text.h"
#include "../fmt/textCursor.h"
#include "../fmt/titleWrap.h"
#include "../fmt/index.h"
#include "../printers.h"

namespace AM5 {

  template<typename P,decltype(Serial)& dev=Serial, typename O=Void>
  struct SerialOutDef:public O {
    using RAW_DEVICE = SerialOutDef<P,dev,O>;
    using Parts=P;
    // using MUST_BE_AT_OUTPUT_BASE=O::OUTPUT_BASE;//or maybe not
    template<typename T>
    inline void raw(T i) {dev.print(i);}
    inline void endl() {dev<<::endl;O::endl();}//we use no viewport
  };

  using SerialParts=DeviceParts<
    Chain<TextAccelPrinter,TextCursorPrinter,ItemPrinter>::To//emit format messages for accel, cursor amd item
    // ,TitlePrinter//emit format messages for titles (fmtTitle)
  >;

  template<template<typename> class N=NavNode>
  using SerialFmt = AM5::Chain<//wrap inner types
    // DebugFmt,//add debug info when enabled
    TextCursorFmt,//signal selected option on text mode
    IndexFmt,//print option index (1-9)
    TextFmt,//text output format
    TitleWrap,//wrap title in []
    TitlePrinter,
    FullPrinter,//print inner then options
    N//flat navigation control (no sub menus)
  >;

};//AM5

template<decltype(Serial)& dev=Serial,typename Parts=AM5::SerialParts>
using SerialOutDev=AM5::SerialOutDef<Parts,Serial>;