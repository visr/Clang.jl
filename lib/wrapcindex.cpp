#include "Index.h"
#include <cstring>
#include <vector>
#include <set>

#define wci_st(rtype) \
  static inline void wci_save_##rtype(rtype i, char* o) \
    { memcpy(o, &i, sizeof(rtype)); } \
  static inline rtype wci_get_##rtype(char* b) \
    { rtype c; memcpy(&c, b, sizeof(rtype)); return c; } \
  extern "C" unsigned int wci_size_##rtype() { return sizeof(rtype); }


// Struct helpers: memcpy shenanigans due to no structs byval
wci_st(CXSourceLocation)
wci_st(CXSourceRange)
wci_st(CXTUResourceUsageEntry)
wci_st(CXTUResourceUsage)
wci_st(CXCursor)
wci_st(CXType)
wci_st(CXToken)
wci_st(CXString)

typedef std::vector<CXCursor> CursorList;
typedef std::set<CursorList*> allcl_t;
allcl_t allCursorLists;

// to traverse AST with cursor visitor
// TODO: replace this with a C container
CXChildVisitResult wci_visitCB(CXCursor cur, CXCursor par, CXClientData data)
{
  printf("cb\n");
  CursorList* cl = (CursorList*)data;
  cl->push_back(cur);
  return CXChildVisit_Continue;
}

extern "C" {
//#include "wrapcindex.h"

unsigned int wci_getChildren(CXCursor cu, CursorList* cl)
{
  printf("getChildren... hash: %x kind: %d\n", clang_hashCursor(cu), cu.kind );
  clang_visitChildren(cu, wci_visitCB, cl);
  return 0;
}

void wci_getCursorFile(CXCursor cu, char* cxsout)
{
  CXSourceLocation loc = clang_getCursorLocation( cu );
  CXFile cxfile;
  clang_getExpansionLocation(loc, &cxfile, 0, 0, 0);
  wci_save_CXString(clang_getFileName(cxfile), cxsout);
}

CursorList* wci_createCursorList()
{
  CursorList* cl = new CursorList();
  allCursorLists.insert(cl);
  return cl;
}

void wci_disposeCursorList(CursorList* cl)
{
  cl->clear();
  allCursorLists.erase(cl);
}

unsigned int wci_sizeofCursorList(CursorList* cl)
{
  return cl->size();
}

void wci_getCLCursor(char* cuout, CursorList* cl, int cuid)
{
  wci_save_CXCursor((*cl)[cuid], cuout);
}

CXCursor test_cu1(CXTranslationUnit tu)
{
  CXCursor cu = clang_getTranslationUnitCursor(tu);
  printf("hash out: %x kind out: %u\n", clang_hashCursor(cu), cu.kind);
  return cu;
}

void test_cu2(CXCursor cu)
{
  printf("hash in: %x kind in: %u\n", clang_hashCursor(cu), cu.kind);
}

} // extern
