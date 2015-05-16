require "gd"
local lfs = require "lfs"
local headfp = assert(io.open(".git/HEAD"))
local branch = headfp:read "*a":match("([^/]-)%s*$")
headfp:close()


local outfile = "playground/images_"..branch..".h"
local out = assert(io.open(outfile,"w"))

out:write [[
/*
 * auto generated image file
 */
]]

local function convertPNG(pngfile)
	local img = assert(gd.createFromPng(pngfile))
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
const ImageInclude _image_%s_opaque PROGMEM = {
	%d,%d,_image_%s_data,-1
};

]]):format(name,table.concat(pixels,","),
		name,width,height,name,img:getTransparent() and rgbToByte(img,img:getTransparent()) or -1,
		name,width,height,name
	))
end

for filename in lfs.dir "images" do
	if filename:match "%.png$" then
		convertPNG("images/"..filename)
	end
end