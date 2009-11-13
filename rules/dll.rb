require "#{File.dirname(__FILE__)}/native_gcc.rb"

# todo: share code with exe.rb

class DllTask < FileTask
	def initialize(work, name, objects, linkflags)
		super(work, name)
		@prerequisites = @objects = objects
		@LINKFLAGS = " -shared -Xlinker --no-undefined" + linkflags
		
		# todo: save linkflags, like CompileGccTask's CFLAGS.
	end
	
	def execute
		sh "g++ #{@objects.join(' ')}#{@LINKFLAGS} -o #{@NAME}"
	end
end

# Compiles C/C++ code into an dynamic-link library.
# Supports GCC on linux and mingw.
class DllWork < NativeGccWork
	private

	def setup3(all_objects)
		llo = @LOCAL_LIBS.collect { |ll| FileTask.new(self, @COMMON_BUILDDIR + ll + ".a") }
		target = @TARGETDIR + "/" + @BUILDDIR + @NAME + DLL_FILE_ENDING
		@TARGET = DllTask.new(self, target, all_objects + llo, @EXTRA_LINKFLAGS)
		@prerequisites += [@TARGET]
 	end
end