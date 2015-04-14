require "gd"

local pngfile, outfile = ...
if not pngfile then 
	print "pass png file name as 1st arg"
	return
end
if not outfile then outfile = pngfile:gsub("%.png", ".h") end

local img = assert(gd.createFromPng(pngfile))
local out = assert(io.open(outfile,"w"))

out:write [[
/*
 * auto generated image file
 */
]]
local floor = math.floor
local min = math.min
local function rgbToByte(img,col)
	local r = min(3,floor(img:red(col)/255*3+.25))
	local g = min(7,floor(img:green(col)/255 * 7+.25))
	local b = min(7,floor(img:blue(col)/255 * 7+.25))
	return r + g * 4 + b * 32
end

local width, height = img:sizeXY()
local name = pngfile:gsub("^.-([^/\\]+)%.png","%1")
local pixels = {}
for y=0,height-1 do
	for x=0,width-1 do
		pixels[#pixels+1] = ("0x%x"):format(rgbToByte(img,img:getPixel(x,y)))
	end
	pixels[#pixels] = pixels[#pixels].."\n"
end
out:write(([[
const unsigned char _image_%s_data[] PROGMEM = {
	%s
};
const ImageInclude _image_%s PROGMEM = {
	%d,%d,_image_%s_data,%d
};
]]):format(name,table.concat(pixels,","),name,width,height,name,img:getTransparent() or -1))
