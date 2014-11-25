// Pure parsing. Calls methods on a Builder (template argument) to actually construct the AST
//
// XXX All parsing methods assume they take ownership of the input string. This lets them reuse
//     parts of it. You will segfault if the input string cannot be reused and written to.

#include <stdio.h>

#include "istring.h"

namespace cashew {

// common strings

extern IString TOPLEVEL,
               DEFUN,
               BLOCK,
               STAT,
               ASSIGN,
               NAME,
               VAR,
               CONDITIONAL,
               BINARY,
               RETURN,
               IF,
               WHILE,
               DO,
               FOR,
               SEQ,
               SUB,
               CALL,
               NUM,
               LABEL,
               BREAK,
               CONTINUE,
               SWITCH,
               STRING,
               INF,
               NaN,
               TEMP_RET0,
               UNARY_PREFIX,
               UNARY_POSTFIX,
               MATH_FROUND,
               SIMD_FLOAT32X4,
               SIMD_INT32X4,
               PLUS,
               MINUS,
               OR,
               AND,
               XOR,
               L_NOT,
               B_NOT,
               LT,
               GE,
               LE,
               GT,
               EQ,
               NE,
               DIV,
               MOD,
               RSHIFT,
               LSHIFT,
               TRSHIFT,
               TEMP_DOUBLE_PTR,
               HEAP8,
               HEAP16,
               HEAP32,
               HEAPF32,
               HEAPU8,
               HEAPU16,
               HEAPU32,
               HEAPF64,
               F0,
               EMPTY;

extern StringSet keywords;

template<class NodeRef, class Builder>
class Parser {

  static bool isSpace(char x) { return x == 32 || x == 9 || x == 10 || x == 13; } /* space, tab, linefeed/newline, or return */
  static char* skipSpace(char* curr) { while (*curr && isSpace(*curr)) curr++; return curr; }

  static bool isIdentInit(char x) { return (x >= 'a' && x <= 'z') || (x >= 'A' && x <= 'Z') || x == '_' || x == '$'; }
  static bool isIdentPart(char x) { return isIdentInit(x) || (x >= '0' && x <= '9'); }

  // An atomic fragment of something. Stops at a natural boundary.
  enum FragType {
    KEYWORD = 0,
    OPERATOR = 1,
    IDENT = 2,
    STRING = 3 // without quotes
  };

  struct Frag {
    IString str;
    int size;
    FragType type;

    Frag(char* src) {
      assert(!isSpace(*src));
      char *start = src;
      if (isIdentInit(*src)) {
        // read an identifier or a keyword
        src++;
        while (isIdentPart(*src)) {
          src++;
        }
        if (*src == 0) {
          str.set(start);
        } else {
          char temp = *src;
          *src = 0;
          str.set(start, false);
          *src = temp;
        }
        type = keywords.has(str) ? KEYWORD : IDENT;
      } else if (*src == '"' || *src == '\'') {
        char *end = strchr(src+1, *src);
        *end = 0;
        str.set(src+1);
        src = end+1;
        type = STRING;
      } else assert(0);
      size = src - start;
    }
  };

  NodeRef parseElement(char*& src, char sep=';') {
    Frag frag(src);
    src += frag.size;
    printf("parseElement frag %s %d\n", frag.str.str, frag.type);
    switch (frag.type) {
      case KEYWORD: {
        return parseAfterKeyword(frag, src, sep);
      }
      case IDENT: {
        return parseAfterIdent(frag, src, sep);
      }
      case STRING: {
        assert(0);
      }
      default: assert(0);
    }
  }

  NodeRef parseAfterKeyword(Frag& frag, char*& src, char sep) {
    assert(0);
  }

  NodeRef parseAfterIdent(Frag& frag, char*& src, char sep) {
    src = skipSpace(src);
    if (*src == ';' || *src == 0) {
      return Builder::makeName(frag.str);
    } else if (*src == '(') {
      return parseCall(frag.str, src);
    }
    assert(0);
  }

  NodeRef parseCall(IString target, char*& src) {
    assert(*src == '(');
    src++;
    NodeRef params = Builder::makeList();
    while (*src != ')') {
      src = skipSpace(src);
      if (*src == ')') break;
      Builder::appendToList(params, parseElement(src, ','));
    }
    return nullptr;
  }

public:
  // Highest-level parsing, as of a JavaScript script file.
  NodeRef parseToplevel(char* src) {
    return parseBlock(src, Builder::makeToplevel());
  }

  // Parses a block of code (e.g. a bunch of statements inside {,}, or the top level of o file)
  NodeRef parseBlock(char* src, NodeRef block=nullptr) {
    if (!block) block = Builder::makeBlock();
    while (*src) {
      src = skipSpace(src);
      if (!*src) break;
      NodeRef element = parseElement(src);
      Builder::appendToBlock(block, element);
    }
    return block;
  }
};

} // namespace cashew

