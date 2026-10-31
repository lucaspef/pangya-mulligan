// JP 983: tinyxmlerror.cpp da TinyXML 2.5.3 (Lee Thomason, licença zlib) como a biblioteca Refresh a embute: tudo dentro de `namespace rf`,
// assert ligado e __FILE__/__LINE__ do 983 (#line; sem assert: deslocamento 0 suposto). Texto original abaixo; mudanças só nas linhas "JP 983". Sessão B144.
#undef NDEBUG	// JP 983: a Refresh foi compilada sem NDEBUG (_wassert no 983)
#include <assert.h>	// JP 983
#line 1 "src\\tinyxml\\tinyxmlerror.cpp"	// JP 983
/*
www.sourceforge.net/projects/tinyxml
Original code (2.0 and earlier )copyright (c) 2000-2006 Lee Thomason (www.grinninglizard.com)

This software is provided 'as-is', without any express or implied 
warranty. In no event will the authors be held liable for any 
damages arising from the use of this software.

Permission is granted to anyone to use this software for any 
purpose, including commercial applications, and to alter it and 
redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must
not claim that you wrote the original software. If you use this
software in a product, an acknowledgment in the product documentation
would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.
*/

#include "refresh/tinyxml/tinyxml.h"	// JP 983
namespace rf {	// JP 983
#line 26 "src\\tinyxml\\tinyxmlerror.cpp"	// JP 983

// The goal of the seperate error file is to make the first
// step towards localization. tinyxml (currently) only supports
// english error messages, but the could now be translated.
//
// It also cleans up the code a bit.
//

const char* TiXmlBase::errorString[ TIXML_ERROR_STRING_COUNT ] =
{
	"No error",
	"Error",
	"Failed to open file",
	"Memory allocation failed.",
	"Error parsing Element.",
	"Failed to read Element name",
	"Error reading Element value.",
	"Error reading Attributes.",
	"Error: empty tag.",
	"Error reading end tag.",
	"Error parsing Unknown.",
	"Error parsing Comment.",
	"Error parsing Declaration.",
	"Error document empty.",
	"Error null (0) or unexpected EOF found in input stream.",
	"Error parsing CDATA.",
	"Error when TiXmlDocument added to document, because TiXmlDocument can only be at the root.",
};
}	// namespace rf (JP 983)
