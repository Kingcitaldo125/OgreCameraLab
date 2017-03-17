#ifndef _EXCEPTION_H_
#define _EXCEPTION_H_



namespace ssurge
{
	/// A general purpose exception class to indicate ssurge-related errors
	class Exception
	{
		/***** ATTRIBUTES *****/
	protected:
		/// The error description
		std::string mDescription;

		/// The file in which the error occurred
		std::string mFile;

		/// The line number on which the error occurred
		unsigned int mLine;

		/***** CONSTRUCTORS / DESTRUCTORS *****/
	public:
		/// The only constructor
		Exception(std::string file, unsigned int line, std::string desc) : mDescription(desc), mFile(file), mLine(line) { }

		/***** METHODS *****/
	public:
		/// Returns an error string
		std::string getString()
		{
			std::stringstream ss;
			ss << "'" << mDescription << "' [" << mFile << "@" << mLine << "]";
			return ss.str();
		}
	};
}

#endif