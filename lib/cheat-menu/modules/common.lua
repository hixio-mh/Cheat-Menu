-- This module contains all common functions used all over the menu

local module = {}

function module.DropDownMenu(label,func)
    if imgui.CollapsingHeader(label) then
        imgui.Spacing()

        func()

        imgui.Spacing()
        imgui.Separator()
        imgui.Spacing()
    end
end

function module.QuickSpawner()

    fcommon.KeyWait(keys.control_key,keys.quickspawner_key)

    memory.write(0x00969110,0,1)
    result = ''
    while not wasKeyPressed(0x0D) do
        if #result > 25 then
            result = ''
        end
        if wasKeyPressed(0x2E) then
            result = ''
        elseif wasKeyPressed(0x08) then
            result = result:sub(1, -2)
        elseif wasKeyPressed(readMemory(0x00969110,1,false)) then
            result = string.format('%s%s',result,memory.tostring(0x00969110,1,false));
        end
        printStringNow(string.format('[%s]',result),1500);
        text = result

        for i = 0,#result,1 do
           local weapon =  fweapons.CBaseWeaponInfo(text)
            if fweapons.tweapons.quick_spawn[0] == true
            and fweapons.CBaseWeaponInfo(text) < 47 and weapon > 0 then
                fweapons.GiveWeaponToPlayer(weapon)
                return
            end

            local vehicle = fvehicles.CBaseModelInfo(text)
            if fvehicles.tvehicles.quick_spawn[0] == true and vehicle > 400 and vehicle < 100000 then
                fvehicles.GiveVehicleToPlayer(vehicle)
                return
            end
            text = text:sub(2)
        end
        wait(0)
    end
end

function module.InformationTooltip(text)
    if imgui.IsItemHovered() then
        imgui.BeginTooltip()
        imgui.Spacing()
        imgui.SetTooltip(text)
        imgui.Dummy(imgui.ImVec2(50,50))
        imgui.EndTooltip()
    end
end

function module.CheatActivated()
    printHelpString("Cheat ~g~Activated")
end

function module.CheatDeactivated()
    printHelpString("Cheat ~r~Deactivated")
end

function module.GetSize(count)
    if count == 1 then
        return ((imgui.GetWindowWidth()- 15*imgui.StyleVar.WindowPadding) / count), (imgui.GetWindowHeight()/25)
    else
        return ((imgui.GetWindowWidth()- 2*imgui.StyleVar.WindowPadding - (count-1)*imgui.StyleVar.ItemSpacing) / count), (imgui.GetWindowHeight()/25)
    end
end

function module.LoadTexture(list,path,model,extention)
    local func_path = path .. tostring(model) .. extention
    local image = imgui.CreateTextureFromFile(func_path)
    list[tostring(model)] = image
end

function IsValidModForVehicle(table)
    if isCharInAnyCar(PLAYER_PED) then
        local CVehicle =  getCarPointer(storeCarCharIsInNoSave(PLAYER_PED))
        if callMethod(0x49B010,CVehicle,2,2,table,CVehicle) == 1 then
             return true
        end
    else
        return false
    end
end

function module.UiCreateButtons(names,funcs)

    imgui.PushStyleVarVec2(imgui.StyleVar.ItemSpacing,imgui.ImVec2(0,0))

    for i=1,#names,1 do
        if cheatMenu.menubuttons.current == i then
            imgui.PushStyleColor(imgui.Col.Button, imgui.ImVec4(0.060,0.530,0.980,1.0))
        end
        if imgui.Button(names[i],imgui.ImVec2(imgui.GetWindowWidth()/4 - 4*imgui.StyleVar.WindowPadding,imgui.GetWindowHeight()/30)) then
            cheatMenu.menubuttons.current = i
        end
        if cheatMenu.menubuttons.current == i then
            imgui.PushStyleColor(imgui.Col.Button, imgui.ImVec4(0.260,0.590,0.980,0.400))
        end

        if i%4 ~= 0 then
            imgui.SameLine()
        end
    end

    imgui.PopStyleVar()
    imgui.Spacing()

    for i=1,#funcs,1 do
        if cheatMenu.menubuttons.current == i then
            imgui.Spacing()
            funcs[i]()
            break
        end
    end
end

function module.ShowEntries(title,model_table,rows,store_table,image_path,image_extention,func_load_model,func_show_tooltip,skip_check)

    for j=1,#model_table,1 do
        if IsValidModForVehicle(model_table[j]) or skip_check == true then
            fcommon.DropDownMenu(title,function()
                imgui.Spacing()

                for i=1,#model_table,1 do

                    if store_table[tostring(model_table[i])] ~= "LOADING"  then

                        if store_table[tostring(model_table[i])] == nil then
                            store_table[tostring(model_table[i])] = "LOADING"
                            lua_thread.create(module.LoadTexture,store_table,image_path,model_table[i],image_extention)
                        else
                            local x,y = fcommon.GetSize(rows)
                            if skip_check == false then
                                if IsValidModForVehicle(model_table[i]) then
                                    if imgui.ImageButton(store_table[tostring(model_table[i])],imgui.ImVec2(x,y)) then
                                        func_load_model(model_table[i])
                                    end
                                end
                            else
                                if imgui.ImageButton(store_table[tostring(model_table[i])],imgui.ImVec2(x/1.2,y*4)) then
                                    func_load_model(model_table[i])
                                end
                            end
                            if func_show_tooltip ~= nil then
                                if imgui.IsItemHovered() then
                                    imgui.BeginTooltip()
                                    imgui.SetTooltip(func_show_tooltip(model_table[i]))
                                    imgui.EndTooltip()
                                end
                            end
                        end
                    end

                    if (i == 1) or (i % rows ~= 0) then
                        imgui.SameLine()
                    end
                end
                imgui.NewLine()
            end)
            break
        end
    end
end

function module.RadioButton(label,rb_table,addr_table)
    local button = imgui.new.int(#addr_table + 1)

    imgui.Text(label)

    for i = 1, #addr_table,1 do
        if readMemory(addr_table[i],1,false) == 1 then
            button[0] = i
        end
        if imgui.RadioButtonIntPtr(rb_table[i],button,i) then
            for j = 1,#addr_table,1 do
                writeMemory(addr_table[j],1,0,false)
            end
            button[0] = i
            writeMemory(addr_table[i],1,1,false)
            module.CheatActivated()
        end
    end

    if imgui.RadioButtonIntPtr("Default " .. label,button,#addr_table + 1) then
        for j = 1,#addr_table,1 do
            writeMemory(addr_table[j],1,0,false)
        end
        module.CheatActivated()
    end
end

function module.RwMemory(address,size,value,protect,is_float)
    if protect ~= true then protect = false end
    if is_float ~= true then is_float = false end

    if value == nil then
        if is_float then
            return memory.getfloat(address,protect)
        else
            return memory.read(address,size,protect)
        end
    else
        if is_float then
            memory.setfloat(address,value,protect)
        else
            memory.write(address,value,size,protect)
        end
    end
end

function module.CheatActivated()
    printHelpString("Cheat ~g~Activated")
end

function module.CheatDeactivated()
    printHelpString("Cheat ~r~Deactivated")
end

function module.CheckBox(arg)
    arg.value = arg.value or 1
    arg.value2 = arg.value2 or 0
    arg.var = arg.var or nil
    arg.func = arg.func or nil

    local func_bool =  arg.var or imgui.new.bool()
    if arg.var == nil then
        if (readMemory(arg.address,1,false)) == arg.value2 then
            func_bool[0] = false
        else
            func_bool[0] = true
        end
    end

    if imgui.Checkbox(arg.name,func_bool) then
        if arg.var == nil then
            if func_bool[0] then
                writeMemory(arg.address,1,arg.value,false)
                module.CheatActivated()
            else
                writeMemory(arg.address,1,arg.value2,false)
                module.CheatDeactivated()
            end
        else
            if arg.var[0] then
                module.CheatActivated()
            else
                module.CheatDeactivated()
            end
            if arg.func ~= nil then arg.func() end
        end
    end

    if arg.help_text ~= nil then
        module.InformationTooltip(arg.help_text)
    end
end



function module.UpdateStat(arg)
    if arg.min == nil then arg.min = 0 end
    if arg.max == nil then arg.max = 1000 end

    module.DropDownMenu(arg.name,function()

        if arg.help_text ~= nil then
            fcommon.InformationTooltip(arg.help_text)
        end

        local change_value = math.floor((arg.max - arg.min)/10)
        local value = imgui.new.int(math.floor(getFloatStat(arg.stat)))

        imgui.Columns(3,nil,false)
        imgui.Text("Max = " .. arg.max)
        imgui.NextColumn()
        imgui.Text("Current = " .. value[0])
        imgui.NextColumn()
        imgui.Text("Min = " .. arg.min)
        imgui.Columns(1)

        imgui.PushItemWidth(imgui.GetWindowWidth()-50)
        if imgui.InputInt("Set",value) then
            setFloatStat(arg.stat,value[0])
        end
        imgui.PopItemWidth()

        if imgui.Button("Increase",imgui.ImVec2(fcommon.GetSize(4))) then
            setFloatStat(arg.stat,(value[0]+change_value))
        end
        imgui.SameLine()
        if imgui.Button("Decrease",imgui.ImVec2(fcommon.GetSize(4))) then
            setFloatStat(arg.stat,(value[0]-change_value))
        end
        imgui.SameLine()
        if imgui.Button("Minimum",imgui.ImVec2(fcommon.GetSize(4))) then
            setFloatStat(arg.stat,arg.min)
        end
        imgui.SameLine()
        if imgui.Button("Maximum",imgui.ImVec2(fcommon.GetSize(4))) then
            setFloatStat(arg.stat,arg.max)
        end
        if value[0] < arg.min then
            setFloatStat(arg.stat,arg.min)
        end
        if value[0] > arg.max then
            setFloatStat(arg.stat,arg.max)
        end
    end)
end

function module.UpdateAddress(arg)
    if arg.min == nil then arg.min = 0 end
    if arg.is_float == nil then arg.is_float = false end

    module.DropDownMenu(arg.name,function()

        local value = imgui.new.int(module.RwMemory(arg.address,arg.size,nil,nil,arg.is_float))

        imgui.Columns(3,nil,false)
        imgui.Text("Max = " .. arg.max)
        imgui.NextColumn()
        imgui.Text("Current = " .. value[0])
        imgui.NextColumn()
        imgui.Text("Min = " .. arg.min)
        imgui.Columns(1)

        imgui.Spacing()

        imgui.PushItemWidth(imgui.GetWindowWidth()-50)
        if imgui.InputInt("Set",value) then
            module.RwMemory(arg.address,arg.size,value[0],nil,arg.is_float)
        end
        imgui.PopItemWidth()

        imgui.Spacing()
        if imgui.Button("Increase",imgui.ImVec2(fcommon.GetSize(4))) and value[0] < arg.max then
            module.RwMemory(arg.address,arg.size,(value[0]+math.floor(arg.max/10)),nil,arg.is_float)
        end
        imgui.SameLine()
        if imgui.Button("Decrease",imgui.ImVec2(fcommon.GetSize(4)))  and value[0] > arg.min then
            module.RwMemory(arg.address,arg.size,(value[0]-math.floor(arg.max/10)),nil,arg.is_float)
        end
        imgui.SameLine()
        if imgui.Button("Maximum",imgui.ImVec2(fcommon.GetSize(4))) then
            module.RwMemory(arg.address,arg.size,arg.max,nil,arg.is_float)
        end
        imgui.SameLine()
        if imgui.Button("Minimum",imgui.ImVec2(fcommon.GetSize(4))) then
            module.RwMemory(arg.address,arg.size,arg.min,nil,arg.is_float)
        end
        imgui.Spacing()
        if value[0] < arg.min then
            module.RwMemory(arg.address,arg.size,arg.min,nil,arg.is_float)
        end

        if value[0] > arg.max then
            module.RwMemory(arg.address,arg.size,arg.max,nil,arg.is_float)
        end
    end)
end

function module.KeyWait(key1,key2)
    while isKeyDown(key1)
    and isKeyDown(key2) do
        wait(0)
    end
end

return module