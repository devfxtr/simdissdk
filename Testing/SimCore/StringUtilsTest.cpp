/* -*- mode: c++ -*- */
/****************************************************************************
 *****                                                                  *****
 *****                   Classification: UNCLASSIFIED                   *****
 *****                    Classified By:                                *****
 *****                    Declassify On:                                *****
 *****                                                                  *****
 ****************************************************************************
 *
 *
 * Developed by: Naval Research Laboratory, Tactical Electronic Warfare Div.
 *               EW Modeling & Simulation, Code 5773
 *               4555 Overlook Ave.
 *               Washington, D.C. 20375-5339
 *
 * License for source code at https://simdis.nrl.navy.mil/License.aspx
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include <iostream>
#include <vector>
#include <string>
#include <limits>
#include "simCore/Common/SDKAssert.h"
#include "simCore/String/Format.h"
#include "simCore/String/Utils.h"
#include "simCore/String/Tokenizer.h"
#include "simCore/Common/Version.h"
#include "simCore/Common/SDKAssert.h"

using namespace std;

namespace
{

int testBefore(const std::string& str, const std::string& needle, const std::string& answer, std::string answerLast="")
{
  if (answerLast.empty()) answerLast = answer;
  int rv = 0;
  if (simCore::StringUtils::before(str, needle) != answer)
  {
    cerr << "Error: before(" << str << "," << needle << ") != " << answer << endl;
    cerr << "   " << simCore::StringUtils::before(str, needle) << endl;
    rv++;
  }
  if (simCore::StringUtils::beforeLast(str, needle) != answerLast)
  {
    cerr << "Error: beforeLast(" << str << "," << needle << ") != " << answerLast << endl;
    cerr << "   " << simCore::StringUtils::beforeLast(str, needle) << endl;
    rv++;
  }
  if (needle.length() == 1)
  {
    if (simCore::StringUtils::before(str, needle[0]) != answer)
    {
      cerr << "Error: before char(" << str << "," << needle << ") != " << answer << endl;
      cerr << "   " << simCore::StringUtils::before(str, needle[0]) << endl;
      rv++;
    }
    if (simCore::StringUtils::beforeLast(str, needle[0]) != answerLast)
    {
      cerr << "Error: beforeLast char(" << str << "," << needle << ") != " << answerLast << endl;
      cerr << "   " << simCore::StringUtils::beforeLast(str, needle[0]) << endl;
      rv++;
    }
  }
  return rv;
}

int testAfter(const std::string& str, const std::string& needle, const std::string& answer, std::string answerLast="")
{
  if (answerLast.empty()) answerLast = answer;
  int rv = 0;
  if (simCore::StringUtils::after(str, needle) != answer)
  {
    cerr << "Error: after(" << str << "," << needle << ") != " << answer << endl;
    cerr << "   " << simCore::StringUtils::after(str, needle) << endl;
    rv++;
  }
  if (simCore::StringUtils::afterLast(str, needle) != answerLast)
  {
    cerr << "Error: afterLast(" << str << "," << needle << ") != " << answerLast << endl;
    cerr << "   " << simCore::StringUtils::afterLast(str, needle) << endl;
    rv++;
  }
  if (needle.length() == 1)
  {
    if (simCore::StringUtils::after(str, needle[0]) != answer)
    {
      cerr << "Error: after char(" << str << "," << needle << ") != " << answer << endl;
      cerr << "   " << simCore::StringUtils::after(str, needle[0]) << endl;
      rv++;
    }
    if (simCore::StringUtils::afterLast(str, needle[0]) != answerLast)
    {
      cerr << "Error: afterLast char(" << str << "," << needle << ") != " << answerLast << endl;
      cerr << "   " << simCore::StringUtils::afterLast(str, needle[0]) << endl;
      rv++;
    }
  }
  return rv;
}

int testSubstitute(const std::string& haystack, const std::string& needle, const std::string& repl, const std::string& answer, bool replaceAll=true)
{
  std::string rv = simCore::StringUtils::substitute(haystack, needle, repl, replaceAll);
  if (rv != answer)
  {
    cerr << "Error: substitute(" << haystack << "," << needle << "," << repl << ") != " << answer << endl;
    cerr << "   " << simCore::StringUtils::substitute(haystack, needle, repl, replaceAll) << endl;
    return 1;
  }
  return 0;
}

int testTrim()
{
  int rv = 0;
  // Trim left
  rv += SDK_ASSERT(simCore::StringUtils::trimLeft("  43") == "43");
  rv += SDK_ASSERT(simCore::StringUtils::trimLeft("  43\t") == "43\t");
  rv += SDK_ASSERT(simCore::StringUtils::trimLeft("  43 ") == "43 ");
  rv += SDK_ASSERT(simCore::StringUtils::trimLeft("43  ") == "43  ");
  rv += SDK_ASSERT(simCore::StringUtils::trimLeft("4 3") == "4 3");
  rv += SDK_ASSERT(simCore::StringUtils::trimLeft(" 4 3 ") == "4 3 ");
  rv += SDK_ASSERT(simCore::StringUtils::trimLeft("43") == "43");
  rv += SDK_ASSERT(simCore::StringUtils::trimLeft("   ") == "");
  rv += SDK_ASSERT(simCore::StringUtils::trimLeft(" ") == "");
  rv += SDK_ASSERT(simCore::StringUtils::trimLeft("") == "");

  // Trim right
  rv += SDK_ASSERT(simCore::StringUtils::trimRight("  43") == "  43");
  rv += SDK_ASSERT(simCore::StringUtils::trimRight("  43\t") == "  43");
  rv += SDK_ASSERT(simCore::StringUtils::trimRight("  43 ") == "  43");
  rv += SDK_ASSERT(simCore::StringUtils::trimRight("43  ") == "43");
  rv += SDK_ASSERT(simCore::StringUtils::trimRight("4 3") == "4 3");
  rv += SDK_ASSERT(simCore::StringUtils::trimRight(" 4 3 ") == " 4 3");
  rv += SDK_ASSERT(simCore::StringUtils::trimRight("43") == "43");
  rv += SDK_ASSERT(simCore::StringUtils::trimRight("   ") == "");
  rv += SDK_ASSERT(simCore::StringUtils::trimRight(" ") == "");
  rv += SDK_ASSERT(simCore::StringUtils::trimRight("") == "");

  // Trim both sides
  rv += SDK_ASSERT(simCore::StringUtils::trim("  43") == "43");
  rv += SDK_ASSERT(simCore::StringUtils::trim("  43\t") == "43");
  rv += SDK_ASSERT(simCore::StringUtils::trim("  43 ") == "43");
  rv += SDK_ASSERT(simCore::StringUtils::trim("43  ") == "43");
  rv += SDK_ASSERT(simCore::StringUtils::trim("4 3") == "4 3");
  rv += SDK_ASSERT(simCore::StringUtils::trim(" 4 3 ") == "4 3");
  rv += SDK_ASSERT(simCore::StringUtils::trim("43") == "43");
  rv += SDK_ASSERT(simCore::StringUtils::trim("   ") == "");
  rv += SDK_ASSERT(simCore::StringUtils::trim(" ") == "");
  rv += SDK_ASSERT(simCore::StringUtils::trim("") == "");
  rv += SDK_ASSERT(simCore::StringUtils::trim("Test\t") == "Test");
  rv += SDK_ASSERT(simCore::StringUtils::trim(" Te st ") == "Te st");
  rv += SDK_ASSERT(simCore::StringUtils::trim("\tTest") == "Test");
  rv += SDK_ASSERT(simCore::StringUtils::trim("\t\rTest\n") == "Test");
  rv += SDK_ASSERT(simCore::StringUtils::trim("\n") == "");
  rv += SDK_ASSERT(simCore::StringUtils::trim("\t") == "");
  rv += SDK_ASSERT(simCore::StringUtils::trim("\r") == "");
  rv += SDK_ASSERT(simCore::StringUtils::trim("\t   \r") == "");
  rv += SDK_ASSERT(simCore::StringUtils::trim("\tTest newline \n in the middle\r") == "Test newline \n in the middle");

  // Irregular whitespace characters
  rv += SDK_ASSERT(simCore::StringUtils::trim("  43", "4") == "  43");
  rv += SDK_ASSERT(simCore::StringUtils::trim("  43\t", "4") == "  43\t");
  rv += SDK_ASSERT(simCore::StringUtils::trim("  43 ", "4") == "  43 ");
  rv += SDK_ASSERT(simCore::StringUtils::trim("43  ", "4") == "3  ");
  rv += SDK_ASSERT(simCore::StringUtils::trim("4 3", "4") == " 3");
  rv += SDK_ASSERT(simCore::StringUtils::trim(" 4 3 ", "4") == " 4 3 ");
  rv += SDK_ASSERT(simCore::StringUtils::trim("43", "4") == "3");
  rv += SDK_ASSERT(simCore::StringUtils::trim("   ", "4") == "   ");
  rv += SDK_ASSERT(simCore::StringUtils::trim(" ", "4") == " ");
  rv += SDK_ASSERT(simCore::StringUtils::trim("", "4") == "");
  rv += SDK_ASSERT(simCore::StringUtils::trimLeft("43", "4") == "3");
  rv += SDK_ASSERT(simCore::StringUtils::trimLeft("43", "3") == "43");
  rv += SDK_ASSERT(simCore::StringUtils::trimRight("43", "4") == "43");
  rv += SDK_ASSERT(simCore::StringUtils::trimRight("43", "3") == "4");

  // More than one whitespace, irregular
  rv += SDK_ASSERT(simCore::StringUtils::trim("43  ", "43") == "  ");
  rv += SDK_ASSERT(simCore::StringUtils::trim("4 3", "34") == " ");
  rv += SDK_ASSERT(simCore::StringUtils::trim(" 4 3 ", "43") == " 4 3 ");
  rv += SDK_ASSERT(simCore::StringUtils::trim("aaaaahah mmmmm", "am") == "hah ");
  rv += SDK_ASSERT(simCore::StringUtils::trim("theThe", "the") == "T");
  rv += SDK_ASSERT(simCore::StringUtils::trim("theThe", "het") == "T");
  rv += SDK_ASSERT(simCore::StringUtils::trim("theThe", "eht") == "T");
  rv += SDK_ASSERT(simCore::StringUtils::trim("// Comment line", "/*# ") == "Comment line");
  rv += SDK_ASSERT(simCore::StringUtils::trim("# Comment line", "/*# ") == "Comment line");
  rv += SDK_ASSERT(simCore::StringUtils::trim("/* Comment line */", "/*# ") == "Comment line");
  rv += SDK_ASSERT(simCore::StringUtils::trim("/*   */", "/*# ") == "");
  return rv;
}

int testEscapeAndUnescape(const std::string& source, const std::string& dest)
{
  int rv = 0;

  std::string shouldMatchDest = simCore::StringUtils::addEscapeSlashes(source);
  rv += SDK_ASSERT(dest == shouldMatchDest);
  std::string shouldMatchSource = simCore::StringUtils::removeEscapeSlashes(shouldMatchDest);
  rv += SDK_ASSERT(source == shouldMatchSource);

  return rv;
}

int testEscape()
{
  int rv = 0;

  // Quotes
  rv += SDK_ASSERT(testEscapeAndUnescape("\"Quote to start", "\\\"Quote to start") == 0);                    // "Quote to start      =>  \"Quote to start
  rv += SDK_ASSERT(testEscapeAndUnescape("\"Quotes ev\"erywhere\"", "\\\"Quotes ev\\\"erywhere\\\"") == 0);  // "Quotes ev"erywhere" =>  \"Quotes ev\"erywhere\"

  // Slashes
  rv += SDK_ASSERT(testEscapeAndUnescape("\\Slash to start", "\\\\Slash to start") == 0);                     // \Slash to start       => \\Slash to start
  rv += SDK_ASSERT(testEscapeAndUnescape("\\Slashes ev\\erywhere\\", "\\\\Slashes ev\\\\erywhere\\\\") == 0); // \Slashes ev\erywhere\ => \\Slahes ev\\erywhere\\    eol

  // Both Quotes and Slashes
  // Both \"slashes" and quotes\   =>    Both \\\"slashes\" and quotes\\    eol
  std::string ans = "Both \\\\";
  ans.append("\\\"slashes\\\" and quotes\\\\");
  rv += SDK_ASSERT(testEscapeAndUnescape("Both \\\"slashes\" and quotes\\", ans) == 0);

  // Real use cases
  rv += SDK_ASSERT(testEscapeAndUnescape("^Test \\(GPS\\)", "^Test \\\\(GPS\\\\)") == 0);                     // ^Test \(GPS\)       =>   ^Test \\(GPS\\)
  rv += SDK_ASSERT(testEscapeAndUnescape("^Test \\(GPS\\)\"", "^Test \\\\(GPS\\\\)\\\"") == 0);               // ^Test \(GPS\)"      =>   ^Test \\(GPS\\)\"

  // Test \n
  rv += SDK_ASSERT(testEscapeAndUnescape("\n", "\\0xA") == 0);
  rv += SDK_ASSERT(testEscapeAndUnescape("\nText", "\\0xAText") == 0);
  rv += SDK_ASSERT(testEscapeAndUnescape("Text\nText", "Text\\0xAText") == 0);
  rv += SDK_ASSERT(testEscapeAndUnescape("Text\n", "Text\\0xA") == 0);

  rv += SDK_ASSERT(testEscapeAndUnescape("\"\n\"", "\\\"\\0xA\\\"") == 0);
  rv += SDK_ASSERT(testEscapeAndUnescape("\" \n\"", "\\\" \\0xA\\\"") == 0);
  rv += SDK_ASSERT(testEscapeAndUnescape("\"\n \"", "\\\"\\0xA \\\"") == 0);
  rv += SDK_ASSERT(testEscapeAndUnescape("\" \n \"", "\\\" \\0xA \\\"") == 0);
  rv += SDK_ASSERT(testEscapeAndUnescape("\"\nText\"", "\\\"\\0xAText\\\"") == 0);
  rv += SDK_ASSERT(testEscapeAndUnescape("\"Text\nText\"", "\\\"Text\\0xAText\\\"") == 0);
  rv += SDK_ASSERT(testEscapeAndUnescape("\"Text\n\"", "\\\"Text\\0xA\\\"") == 0);

  return rv;
}

int testToNativeSeparators()
{
  int rv = 0;

#ifdef WIN32
  rv += SDK_ASSERT(simCore::toNativeSeparators("./test/file") == ".\\test\\file");
  rv += SDK_ASSERT(simCore::toNativeSeparators("./test\\file") == ".\\test\\file");
  rv += SDK_ASSERT(simCore::toNativeSeparators(".\\test\\file") == ".\\test\\file");
  rv += SDK_ASSERT(simCore::toNativeSeparators(".\\test/file") == ".\\test\\file");
  rv += SDK_ASSERT(simCore::toNativeSeparators("c:\\test\\file") == "c:\\test\\file");
  rv += SDK_ASSERT(simCore::toNativeSeparators("c:/test/file") == "c:\\test\\file");
  rv += SDK_ASSERT(simCore::toNativeSeparators("/test/file") == "\\test\\file");
  rv += SDK_ASSERT(simCore::toNativeSeparators("/test/path/") == "\\test\\path\\");
  rv += SDK_ASSERT(simCore::toNativeSeparators("/test/path\\\\") == "\\test\\path\\");
  rv += SDK_ASSERT(simCore::toNativeSeparators("c:/test/\\/file") == "c:\\test\\file");
  rv += SDK_ASSERT(simCore::toNativeSeparators("c:\\/test//file") == "c:\\test\\file");
  rv += SDK_ASSERT(simCore::toNativeSeparators("c:\\/test///file") == "c:\\test\\file");
  rv += SDK_ASSERT(simCore::toNativeSeparators("c:\\/test////file") == "c:\\test\\file");
  rv += SDK_ASSERT(simCore::toNativeSeparators("c:\\/test/////file") == "c:\\test\\file");
  rv += SDK_ASSERT(simCore::toNativeSeparators("c:\\/test\\\\file") == "c:\\test\\file");
  rv += SDK_ASSERT(simCore::toNativeSeparators("c:\\/test\\\\\\file") == "c:\\test\\file");
  rv += SDK_ASSERT(simCore::toNativeSeparators("c:\\/test\\\\\\\\file") == "c:\\test\\file");
#else
  rv += SDK_ASSERT(simCore::toNativeSeparators("./test/file") == "./test/file");
  rv += SDK_ASSERT(simCore::toNativeSeparators("./test\\file") == "./test/file");
  rv += SDK_ASSERT(simCore::toNativeSeparators(".\\test\\file") == "./test/file");
  rv += SDK_ASSERT(simCore::toNativeSeparators(".\\test/file") == "./test/file");
  // Note that Linux does not attempt to correct "C:/" and leaves it in
  rv += SDK_ASSERT(simCore::toNativeSeparators("c:\\test\\file") == "c:/test/file");
  rv += SDK_ASSERT(simCore::toNativeSeparators("c:/test/file") == "c:/test/file");
  rv += SDK_ASSERT(simCore::toNativeSeparators("/test/file") == "/test/file");
  rv += SDK_ASSERT(simCore::toNativeSeparators("/test/path/") == "/test/path/");
  rv += SDK_ASSERT(simCore::toNativeSeparators("/test/path\\\\") == "/test/path/");
  rv += SDK_ASSERT(simCore::toNativeSeparators("c:/test/\\/file") == "c:/test/file");
  rv += SDK_ASSERT(simCore::toNativeSeparators("c:\\/test//file") == "c:/test/file");
  rv += SDK_ASSERT(simCore::toNativeSeparators("c:\\/test///file") == "c:/test/file");
  rv += SDK_ASSERT(simCore::toNativeSeparators("c:\\/test////file") == "c:/test/file");
  rv += SDK_ASSERT(simCore::toNativeSeparators("c:\\/test/////file") == "c:/test/file");
  rv += SDK_ASSERT(simCore::toNativeSeparators("c:\\/test\\\\file") == "c:/test/file");
  rv += SDK_ASSERT(simCore::toNativeSeparators("c:\\/test\\\\\\file") == "c:/test/file");
  rv += SDK_ASSERT(simCore::toNativeSeparators("c:\\/test\\\\\\\\file") == "c:/test/file");
#endif
  // URLs should not get mangled in any way
  rv += SDK_ASSERT(simCore::toNativeSeparators("https://simdis.nrl.navy.mil/jira") == "https://simdis.nrl.navy.mil/jira");
  rv += SDK_ASSERT(simCore::toNativeSeparators("https://simdis.nrl.navy.mil\\jira") == "https://simdis.nrl.navy.mil\\jira");
  rv += SDK_ASSERT(simCore::toNativeSeparators("file:///home/test/file") == "file:///home/test/file");
  rv += SDK_ASSERT(simCore::toNativeSeparators("file:///home/test\\file") == "file:///home/test\\file");
  return rv;
}

int testBeforeAfter()
{
  int rv = 0;
  // Standard case
  rv += SDK_ASSERT(testBefore("foobar=baz", "=", "foobar") == 0);
  rv += SDK_ASSERT(testAfter("foobar=baz", "=", "baz") == 0);
  // Bound 0
  rv += SDK_ASSERT(testBefore("=baz", "=", "") == 0);
  rv += SDK_ASSERT(testAfter("=baz", "=", "baz") == 0);
  // Bound last
  rv += SDK_ASSERT(testBefore("foobar=", "=", "foobar") == 0);
  rv += SDK_ASSERT(testAfter("foobar=", "=", "") == 0);
  // Bound outside
  rv += SDK_ASSERT(testBefore("foobar", "=", "foobar") == 0);
  rv += SDK_ASSERT(testAfter("foobar", "=", "") == 0);
  // Bound double
  rv += SDK_ASSERT(testBefore("foobar=baz=zoo", "=", "foobar", "foobar=baz") == 0);
  rv += SDK_ASSERT(testAfter("foobar=baz=zoo", "=", "baz=zoo", "zoo") == 0);
  // Multi-char delimiter
  rv += SDK_ASSERT(testBefore("foobar:;:baz:;:zoo", ":;:", "foobar", "foobar:;:baz") == 0);
  rv += SDK_ASSERT(testAfter("foobar:;:baz:;:zoo", ":;:", "baz:;:zoo", "zoo") == 0);
  return rv;
}

int testSubstitute()
{
  int rv = 0;
  // Simple substitute
  rv += SDK_ASSERT(testSubstitute("foobar", "bar", "baz", "foobaz") == 0);
  // Double substitute
  rv += SDK_ASSERT(testSubstitute("barfoobar", "bar", "baz", "bazfoobaz") == 0);
  // Substitute with needle in replacement pattern
  rv += SDK_ASSERT(testSubstitute("barbara", "bar", "xxxxbary", "xxxxbaryxxxxbarya") == 0);
  // Single substitution
  rv += SDK_ASSERT(testSubstitute("barbara", "bar", "zoo", "zoobara", false) == 0);
  return rv;
}

int checkStrings(const std::string& expected, const std::string& str)
{
  if (expected != str)
  {
    std::cerr << "Strings do not match: >" << str << "< -- expected: >" << expected << "<" << std::endl;
    return 1;
  }
  return 0;
}

int checkStrings2(const std::string& option1, const std::string& option2, const std::string& str)
{
  if (option1 == str || option2 == str)
    return 0;
  std::cerr << "Strings do not match: >" << str << "< -- expected: >" << option1 << "< or >" << option2 << "<" << std::endl;
  return 1;
}

int buildFormatStrTest()
{
  int rv = 0;
  // Scientific tests -- different build systems give different e+00 or e+000 results
  rv += SDK_ASSERT(0 == checkStrings2("1.52e+025", "1.52e+25", simCore::buildString("", 1.52103484e25, 0, 2, "", false)));
  rv += SDK_ASSERT(0 == checkStrings2("-1.52e+025", "-1.52e+25", simCore::buildString("", -1.52103484e25, 0, 2, "", false)));
  rv += SDK_ASSERT(0 == checkStrings2("1.52e-025", "1.52e-25", simCore::buildString("", 1.52103484e-25, 0, 2, "", false)));
  rv += SDK_ASSERT(0 == checkStrings2("-1.52e-025", "-1.52e-25", simCore::buildString("", -1.52103484e-25, 0, 2, "", false)));
  // Regular tests
  rv += SDK_ASSERT(0 == checkStrings("0", simCore::buildString("", 0.0, 0, 0, "", false)));
  rv += SDK_ASSERT(0 == checkStrings("15.21", simCore::buildString("", 1.52103484e1, 0, 2, "", false)));
  rv += SDK_ASSERT(0 == checkStrings("-15.21", simCore::buildString("", -1.52103484e1, 0, 2, "", false)));
  // NaN and inf tests
  rv += SDK_ASSERT(0 == checkStrings("NaN", simCore::buildString("", std::numeric_limits<double>::quiet_NaN(), 0, 2, "", false)));
  rv += SDK_ASSERT(0 == checkStrings("inf", simCore::buildString("", std::numeric_limits<double>::infinity(), 0, 2, "", false)));
  return rv;
}

}

int StringUtilsTest(int argc, char* argv[])
{
  simCore::checkVersionThrow();
  int rv = 0;

  rv += SDK_ASSERT(testBeforeAfter() == 0);
  rv += SDK_ASSERT(testSubstitute() == 0);

  // Test trimming methods (trimLeft, trimRight, trim)
  rv += SDK_ASSERT(testTrim() == 0);

  // Test adding escape slashes
  rv += SDK_ASSERT(testEscape() == 0);

  // Test the to-native-path code
  rv += SDK_ASSERT(testToNativeSeparators() == 0);

  // buildString() testing
  rv += SDK_ASSERT(buildFormatStrTest() == 0);

  std::cout << "simCore StringUtilsTest " << ((rv == 0) ? "passed" : "failed") << std::endl;

  return rv;
}