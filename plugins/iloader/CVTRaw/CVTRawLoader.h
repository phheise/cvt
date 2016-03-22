#include <cvt/gfx/ILoader.h>
#include <cvt/util/String.h>

namespace cvt {
	class CVTRawLoader : public ILoader
	{
		public:
			CVTRawLoader() {}
			void load( Image& dst, const String& file );
			const String& extension( size_t ) const { return _extension; }
			size_t sizeExtensions() const { return 1; }
			const String& name() const { return _name; }

		private:
			static String _name;
			static String _extension;
	};
}
