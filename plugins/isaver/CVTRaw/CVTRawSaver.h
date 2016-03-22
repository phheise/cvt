#include <cvt/gfx/ISaver.h>
#include <cvt/util/String.h>

namespace cvt {
	class CVTRawSaver : public ISaver
	{
		public:
			CVTRawSaver() {}
			void save( const String& path, const Image& img );
			const String& extension( size_t ) const { return _extension; }
			size_t sizeExtensions() const { return 1; }
			const String& name() const { return _name; }

		private:
			static String _name;
			static String _extension;
	};
}
