#if !defined(COOL_LANG_H)
#define COOL_LANG_H

#include <coolprj/cooledit.h>
#include <vector>

//
// class TParserSelector
// ~~~~~ ~~~~~~~~~~~~~~~
//
class _COOLCLASS TParserSelector {
  public:
    enum TParserInfoEnum{ pieNone, pieDynamic,};
    struct TParserInfo {
      LPCTSTR        Pattern;
      TParserCreator Func;
      owl::uint           Flags;
    };
  public:
    TParserSelector(bool initDefTypes = true);
    ~TParserSelector();
    static bool MatchFileType(LPCTSTR tplFilter, LPCTSTR path);

    TSyntaxParser* CreateParser(TCoolTextWnd* parent, LPCTSTR filename);
    TParserInfo*   MatchParser(LPCTSTR filename);

    void AddParser(LPCTSTR pattern, TParserCreator func);

  protected:
    typedef std::vector<TParserInfo*> TParserInfoArray;
    TParserInfoArray Parsers;

    // default parsers
    static TParserInfo ParserInfo[];
};

// inline for now
inline TSyntaxParser* ChooseSyntaxParser(TCoolTextWnd* parent,LPCTSTR filename)
{
  TParserSelector selector;
  return selector.CreateParser(parent, filename);
}

#endif  // COOL_LANG_H sentry.
