--author : booirror[at]163.com
local LinkManager = {}

local LevelMap = require("src.app.data.LevelMap")
local itemsArray = nil
local gameScene = nil

function LinkManager:init(scene)
    gameScene = scene
    itemsArray = gameScene.itemsArray
end

function LinkManager:canLink(item1, item2)
    if item1:getID() ~= item2:getID() then
        return false
    end
    local r1, c1 = item1:getRow(), item1:getCol()
    local r2, c2 = item2:getRow(), item2:getCol()
    if self:canLinkWithNoCorner(r1, c1, r2, c2) then
        return true, {{r1, c1}, {r2, c2}}
    end
    local b, t = self:canLinkWith1Corner(r1, c1, r2, c2)
    if b then
        return true, {{r1, c1}, t, {r2, c2}}
    end
    b, t = self:canLinkWith2Corner(r1, c1, r2, c2)
    if b then
        return true, {{r1, c1}, t[1], t[2], {r2, c2}}
    end
    return false
end

function LinkManager:isWin()
    local sz = 0;
    for k, v in pairs(itemsArray) do
        sz = 1
    end
    return sz == 0
end

function LinkManager:canPass(row, col)
    if row < 0 or row > LevelMap.maxRow-1 then return true end
    if col < 0 or col > LevelMap.maxCol-1 then return true end
    return gameScene:getItem(row, col) == nil
end

function LinkManager:canLinkWithNoCorner(r1, c1, r2, c2)
    if r1 ~= r2 and c1 ~= c2 then return false end
    if r1 == r2 then
        local dc = c1 > c2 and -1 or 1
        local c = c1 + dc
        while (c ~= c2) do
            if not self:canPass(r1, c) then
                return false
            end
            c = c + dc
        end
    else
        local dr = r1 > r2 and -1 or 1
        local r = r1 + dr
        while r ~= r2 do
            if not self:canPass(r, c1) then
                return false
            end
            r = r + dr
        end
    end
    return true
end

function LinkManager:canLinkWith1Corner(r1, c1, r2, c2)
    local bpass = false
    if r1 == r2 or c1 == c2 then
        return false
    end
    if self:canPass(r1, c2) then
        bpass = self:canLinkWithNoCorner(r1, c1, r1, c2) and
            self:canLinkWithNoCorner(r1, c2, r2, c2)
    end
    if bpass then
        return true, {r1, c2}
    end
    if self:canPass(r2, c1) then
        bpass = self:canLinkWithNoCorner(r1, c1, r2, c1) and
            self:canLinkWithNoCorner(r2, c1, r2, c2)
    end
    if bpass then
        return true, {r2, c1}
    end
    return false
end

function LinkManager:canLinkWith2Corner(r1, c1, r2, c2)
    local tempr = r1
    while tempr ~= -2 do
        tempr = tempr - 1
        if self:canPass(tempr, c1) then
            local b, t = self:canLinkWith1Corner(tempr, c1, r2, c2)
            if b then
                return true, {{tempr, c1}, {t[1], t[2]}}
            end
        else
            break
        end
    end
    tempr = r1
    while tempr ~= LevelMap.maxRow+2 do
        tempr = tempr + 1
        if self:canPass(tempr, c1) then
            local b, t = self:canLinkWith1Corner(tempr, c1, r2, c2)
            if b then
                return true, {{tempr, c1}, {t[1], t[2]}}
            end
        else
            break
        end
    end
    local tempc = c1
    while tempc ~= -2 do
        tempc = tempc - 1
        if self:canPass(r1, tempc) then
            local b, t = self:canLinkWith1Corner(r1, tempc, r2, c2)
            if b then
                return true, {{r1, tempc}, {t[1], t[2]}}
            end
        else
            break
        end
    end
    tempc = c1
    while tempc ~= LevelMap.maxCol+2 do
        tempc = tempc + 1
        if self:canPass(r1, tempc) then
            local b, t = self:canLinkWith1Corner(r1, tempc, r2, c2)
            if b then
                return true, {{r1, tempc}, {t[1], t[2]}}
            end
        else
            break
        end
    end
    return false
end

return LinkManager