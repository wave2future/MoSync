
def default(constant, value)
	s = ("if(defined?(@#{constant.to_s}) == nil) then @#{constant.to_s} = #{value.inspect} end")
	eval(s)
end

class String
	def ext(newEnd)
		doti = rindex('.')
		slashi = rindex('/')
		if(doti && slashi) then if(slashi > doti) then
			return self + newEnd
		end end
		return self[0, doti] + newEnd
	end
	
	def getExt
		doti = rindex('.')
		slashi = rindex('/')
		if(doti && slashi) then
			if(slashi > doti) then
				return nil
			end
		end
		return self[doti..self.length]
	end
end

def sh(cmd)
	#TODO: optimize by removing the extra shell
	#the Process class should be useful.
	puts cmd
	if(!system(cmd)) then
		error "Command failed: '#{$?}'"
	end
end