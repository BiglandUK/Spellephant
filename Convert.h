 #include <iostream>
 #include <sstream>
 #include <string>
 #include <typeinfo>
 #include <stdexcept>
 
 extern std::wstring ToWString(const std::string& s);
 
 class BadConversion : public std::runtime_error {
 public:
   BadConversion(const std::string& s)
     : std::runtime_error(s)
     { }
 };
 
 template<typename T>
 inline std::wstring stringify(const T& x)
 {
   std::ostringstream o;
   if (!(o << x))
     throw BadConversion(std::string("stringify(")
                         + typeid(x).name() + ")");
   return ToWString(o.str());
 }
// 06	   stringstream StrStream;
//07	   StrStream << No;
//08	   string MyString = StrStream.str();