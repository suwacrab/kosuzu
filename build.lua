local function execa(...)
	local cmd = table.concat({...}," ")
	local succ = os.execute(cmd)
	if not succ then
		error(('command "%s" failed'):format(cmd))
	end
end

for _,word in next,arg do
	if word == "build_lib" then
		print("building library")
		execa("make -f mkfile_lib.mak clean")
		execa("make -f mkfile_lib.mak all")
		execa('cp bin\\libkosuzu.a lib\\')
	elseif word == "build_tests" then
		print("building tests...")
		execa("make -f mkfile_tst.mak clean")
		execa("make -f mkfile_tst.mak all")
	end
end

