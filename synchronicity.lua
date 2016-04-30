
local newenv = table.Copy(_G)
newenv["realg"] = _G
newenv["SN"] = table.Copy(SN)

setfenv(1, newenv)

SN.OnReceiveEyeAngleX = function( ent, fang ) -- todo

end

SN.OnReceiveEyeAngleY = function( ent, fang ) -- todo

end


SN.OnCreateMove = function( cmd )
	
	if(cmd:GetCommandNumber() == 0) then return end

	Msg("CMDNUM: ")
	Msg(tostring(cmd:GetCommandNumber()))
	Msg("\n")

	if(SN.IsDrawingLoadingScreen()) then return end
	if(not SN.aimbot) then return end

	local bestdist = 9999999999999999999
	local bestvec = nil
	local besttarget = nil

	local localply = SN.GetLocalPlayer()
	local localindex = SN.GetLocalPlayerIndex()
	local mypos = localply:GetPos()
	local myeyepos = localply:GetAbsEyePos()
	local maxplayers = SN.GetMaxPlayers()

	for i=1, maxplayers do

		local ent = SN.GetEntity(i)
		
		if(i == localindex or not ent) then continue end
		local vec = SN.GetTarget(myeyepos, ent)

		if(not vec) then continue end
		local hispos = ent:GetPos()
		if(not hispos) then continue end

		local dist = hispos:Distance(mypos)
		
		if(bestdist > dist) then
			bestdist = dist
			bestvec = vec
			besttarget = ent

			local ply = ent:ToPlayer()

			if(ply) then
				Msg(dist)
				Msg( ":" )
				Msg(ply:GetNick())
				Msg("\n")
			end

		end

	end

	if(bestvec) then
		print("AIMIN AT: " .. besttarget:EntIndex())
		local ang 
		local vecaimpos = bestvec - myeyepos
		ang = SN.VectorAngles(vecaimpos)
		SN.NormalizeAngles(ang)

		SN.SetViewAngles(ang)
	end

end



SN.OnRunString = function( filename, stringToRun, run, showErrors )
	print("Running: " .. filename )
end


SN.HookTable = {}
SN.HookTable["OnEngineSpew"] = SN.OnEngineSpew
SN.HookTable["OnRunString"] = SN.OnRunString
SN.HookTable["OnCreateMove"] = SN.OnCreateMove
SN.HookTable["OnReceiveEyeAngleX"] = SN.OnReceiveEyeAngleX
SN.HookTable["OnReceiveEyeAngleY"] = SN.OnReceiveEyeAngleY

function SN.OnHookCall(hookname, a1, a2, a3, a4, a5, a6, a7, a8, a9)


	SN.SetHookFunc(SN.OnHookCall)

	if(!hookname) then
		return
	end
	
	local func = SN.HookTable[hookname]

	if(!func) then
		print("!FUNC: " .. hookname)
		return
	end

	local succ, reta, retb, retc, retd, rete, retf, retg = pcall(func, a1, a2, a3, a4, a5, a6, a7, a8, a9)
	if(not succ) then
		print("!HOOK CALL: " .. reta)
		return
	end

	return reta, retb, retc, retd, rete, retf, retg

end

local vecfunc = Vector
local angfunc = Angle
SN.SetHookFunc(SN.OnHookCall, vecfunc, angfunc)
SN.AddBoneUpdate("Player")
SN.AddBoneUpdate("NPC")
SN.AddBoneUpdate("NextBot")


SN.SetSpeed = function(speed)

end

SN.SetSendPacket = function(send)

end


--timer.Create("kek", 0, 0, function()
--	SN.SetVisualRecoil(true)
--end)

concommand.Add("disconnect_cool", function(p,c,a,s)
	SN.Disconnect(s)
end)

concommand.Add("lua_run_cool1", function(p,c,a,s)
	SN.RunString(s, true)
end)

concommand.Add("lua_run_cool2", function(p,c,a,s)
	SN.RunString(s, false)
end)

SN.Aim_RateDanger = function(ourpos, enemy, targetmode)
end

function SN.GetTarget(localeyepos, ent)

	
	if(not ent:IsAlive() ) then return false end
	--if(ent:IsDormant()) then return false end

	local targetvec = SN.CalculateAimVector(ent, localeyepos, 0)
	
	if(targetvec) then
		return targetvec
	end

	return false
end

concommand.Add("lua_coolbot", function(p,c,a,s)

	if(not SN.aimbot) then
		print("ENABLED AIMBUTT")
	else
		print("DISABLED AIMBUTT")
	end

	SN.aimbot = !SN.aimbot
end)

hook.Add("DrawOverlay", "esp", function()
	if(not SN.IsInGame() or SN.IsDrawingLoadingScreen()) then return end

	local localply = SN.GetLocalPlayer()
	local localindex = SN.GetLocalPlayerIndex()
	local maxplayers = SN.GetMaxPlayers()

	for i=1, maxplayers do
		if(i==localindex) then continue end

		local ent = SN.GetEntity(i)
		if(not ent) then continue end

		local ply = ent:ToPlayer()
		if(not ply) then print("not 2player") continue end

		pos = ply:GetAbsPos()

		if(pos.x == 0 and pos.y == 0 and pos.z == 0) then print("no pos") continue end

		local screen = SN.WorldToScreen(pos)

		if(!screen) then print("no screen") continue end

		surface.SetFont( "Default" )


		if(ent:IsAlive()) then
			if(localply:GetTeam() == ply:GetTeam()) then
				surface.SetTextColor( 0,255,0,255 )
			else
				surface.SetTextColor( 255,0,0,255 )
			end
		else
			surface.SetTextColor(0,0,0,255)
		end
			
		surface.SetTextPos( screen.x, screen.y )
		surface.DrawText( ply:GetNick() )

		surface.SetTextPos( screen.x, screen.y + 10 )
		surface.DrawText( ply:GetHealth() )
	end

end)


-- menu shit
surface.CreateFont("buttonfont", {
	font="DermaLarge",
	size=18,
	weight=800,
	antialias=true,
})

surface.CreateFont("closebuttonfont", {
	font="DermaLarge",
	size=15,
	weight=800,
	antialias=true,
})

surface.CreateFont("titlefont", {
	font="ChatFont",
	size=20,
	weight=1200,
	antialias=true,
	italic=false,
})


local RegisterButton = {}
function RegisterButton:Paint(w, h)
	if ( self.Depressed || self.m_bSelected ) then
		surface.SetDrawColor(30, 200, 43)

		local slash1 = 28

		surface.DrawLine(-1000, slash1, 1000, slash1)
		surface.DrawLine(-1000, slash1+1, 1000, slash1+1)
		surface.DrawLine(-1000, slash1+2, 1000, slash1+2)

		return self:SetTextColor(Color( 58, 169, 43 ))
	end

	if ( self.Hovered )	then

		surface.SetDrawColor(80, 169, 43)

		local slash1 = 28

		surface.DrawLine(-1000, slash1, 1000, slash1)
		surface.DrawLine(-1000, slash1+1, 1000, slash1+1)
		surface.DrawLine(-1000, slash1+2, 1000, slash1+2)

		return self:SetTextColor(Color(58, 169, 43 ))
	end

	return self:SetTextColor(Color(158, 162, 167))
end

vgui.Register("RegisterButton", RegisterButton, "DButton")

function SN.ToggleMenu()

	if(SN.menu) then
		SN.menu:SetVisible(false)
		SN.menu:Remove()
		SN.menu = nil
		return
	end

	SN.menu = vgui.Create("DPanel")
	SN.menu:SetSize(325,390)

	SN.menu.Paint = function( pnl, w, h)
		--draw.RoundedBox( 8, 0, 0, w, h, Color( 50, 58,69 ) )
		surface.SetDrawColor( 50, 58, 69 )
		surface.DrawRect(0, 0, w, h)

	end

	SN.menu.Think = function(pnl)
		local mousex = math.Clamp( gui.MouseX(), 1, ScrW()-1 )
		local mousey = math.Clamp( gui.MouseY(), 1, ScrH()-1 )
		
		if(pnl.Dragging) then
			local x = mousex - pnl.Dragging[1]
			local y = mousey - pnl.Dragging[2]

			x = math.Clamp( x, 0, ScrW() - pnl:GetWide() )
			y = math.Clamp( y, 0, ScrH() - pnl:GetTall() )

			pnl:SetPos( x, y )
		end

	end

	SN.menu.OnMousePressed = function(pnl)
		if (gui.MouseY() < (pnl.y + 24) ) then
			pnl.Dragging = { gui.MouseX() - pnl.x, gui.MouseY() - pnl.y }
			pnl:MouseCapture( true )
			return
		end
	end

	SN.menu.OnMouseReleased = function(pnl)
		pnl.Dragging = nil
		pnl:MouseCapture( false )
	end

	SN.menu:Center()
	SN.menu:MakePopup()

	SN.menu.labelTitle = vgui.Create("DLabel", SN.menu)
	SN.menu.labelTitle:SetText("Synchronicity")
	SN.menu.labelTitle:SetFont("titlefont")
	SN.menu.labelTitle:SetSize(SN.menu:GetSize(), 20)
	SN.menu.labelTitle:SetPos(1, 5)
	SN.menu.labelTitle.Paint = function(pnl, w, h)
		surface.SetDrawColor(255, 255, 255)
		surface.DrawLine(-1000, h-1, 1000,h-1)
	end

	SN.menu.btnClose = vgui.Create("DButton", SN.menu)
	SN.menu.btnClose:SetText("X")
	SN.menu.btnClose:SetFont("closebuttonfont")
	SN.menu.btnClose:SetPos( SN.menu:GetWide() - 31 - 4, 0)
	SN.menu.btnClose:SetSize(31, 31)
	SN.menu.btnClose:SetCursor("hand")
	SN.menu.btnClose:SetDrawBorder(true)
	SN.menu.btnClose.DoClick = function(pnl, w, h)
		SN.menu:SetVisible(false)
		SN.menu:Remove()
		SN.menu = nil
	end

	SN.menu.btnClose.Paint = function(pnl, w, h)
		if ( pnl.Depressed || pnl.m_bSelected ) then
			return pnl:SetTextColor(Color( 255, 0, 0 ))
		end

		if ( pnl.Hovered )	then
			return pnl:SetTextColor(Color(255, 100, 0 ))
		end

		return pnl:SetTextColor(Color(255, 50, 0))
	end

	SN.menu.btnAimbot = vgui.Create("RegisterButton", SN.menu)
	SN.menu.btnAimbot:SetPos(30, 30)
	SN.menu.btnAimbot:SetSize(55, 35)
	SN.menu.btnAimbot:SetText("Aimbot")
	SN.menu.btnAimbot:SetFont("buttonfont")

	SN.menu.btnEsp = vgui.Create("RegisterButton", SN.menu)
	SN.menu.btnEsp:SetPos( 95, 30)
	SN.menu.btnEsp:SetSize(55, 35)
	SN.menu.btnEsp:SetText("Visuals")
	SN.menu.btnEsp:SetFont("buttonfont")

	SN.menu.btnWeapon = vgui.Create("RegisterButton", SN.menu)
	SN.menu.btnWeapon:SetPos(160, 30)
	SN.menu.btnWeapon:SetSize(60, 35)
	SN.menu.btnWeapon:SetText("Weapon")
	SN.menu.btnWeapon:SetFont("buttonfont")


	SN.menu.btnMisc = vgui.Create("RegisterButton", SN.menu)
	SN.menu.btnMisc:SetPos(230, 30)
	SN.menu.btnMisc:SetSize(50, 35)
	SN.menu.btnMisc:SetText("Misc")
	SN.menu.btnMisc:SetFont("buttonfont")

	SN.menu.pnlAimbot = vgui.Create("DPanel", SN.menu)
	SN.menu.pnlAimbot:SetSize(300, 290)
	SN.menu.pnlAimbot:SetPos(13, 80)
	SN.menu.pnlAimbot.Paint = function(pnl, w,h)
		
		surface.SetDrawColor(255,255,255)
		surface.DrawOutlinedRect(0,0, w, h)
	end

	SN.menu.checkAimbot = vgui.Create("DCheckBoxLabel", SN.menu.pnlAimbot)
	SN.menu.checkAimbot:SetSize(15,15)
	SN.menu.checkAimbot:SetPos(15,15)
	SN.menu.checkAimbot:SetText("Aimbot")
	SN.menu.checkAimbot:SetValue(SN.aimbot)
	SN.menu.checkAimbot.OnChange = function(pnl, val)

		if(val) then
			SN.aimbot = true
		else
			SN.aimbot = false
		end

	end

	SN.menu.checkAimbot:SizeToContents()

end

concommand.Add("lua_open",  SN.ToggleMenu)