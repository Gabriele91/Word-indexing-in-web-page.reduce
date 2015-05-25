-- init web size conteiner
website=WebSite()
-- download pages
table_indexs={
              'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 
              'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 
              's', 't', 'u', 'v', 'w', 'x', 'y', 'z'   
              }
-- pool
pool=ThreadPool(4)

for k,v in pairs(table_indexs) do
    page=GetHTTP("http://www.treccani.it/enciclopedia/elenco-opere/Enciclopedia_della_Scienza_e_della_Tecnica/" .. v .. "/")
    print("download page "..v)
    --parse a page
    pageHTML=ParserHTML(page)
    -- get links
    links = pageHTML:get_links()
    -- count download pages
    count=0
    -- for all links
    for _,link in pairs(links) do 
        s_start, _ = string.find(link, "_(Enciclopedia_della_Scienza_e_della_Tecnica)",1,true)
        if s_start~=nil then count=count+1 end 
        if s_start~=nil and s_start>=1 then
            local new_link="http://www.treccani.it"..link
            -- add a html page
            pool:enqueue(website,new_link)
        end
    end
    print("added "..count.." pages to download queue")
    count=0
end
--
last_count=pool:task_count()+1
--
repeat
    if(last_count > pool:task_count()) then
		last_count = pool:task_count()
		system_clear()
		print("wait... ["..last_count.."]")
	end
until last_count <= 0

print(website:size())
website:save_to_file("lua_treccani.pages")

