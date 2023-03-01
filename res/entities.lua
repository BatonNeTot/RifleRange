return {
    prototypes = {
        {
            name = 'manager',
            onCreate = function()
                -- Ini
                Ini.setIniFile('input.txt')
                local CountTarget = Ini.getInteger(nil, 'CountTarget', 30)
                local Speed = Ini.getInteger(nil, 'Speed', 200)
                local Time = Ini.getInteger(nil, 'Time', 20)

                -- Background
                local cloudCount = 4
                for i=1, cloudCount do
                    Entities.create('cloud')
                end

                local roomWidth, roomHeight = Window.getRoom()
                local wallWidth, wallHeight = Sprites.getSprite('blockWall'):getSize()
                _G.wallWidth = wallWidth

                local block = Entities.create('blockWall')
                block:setPosition(0, 0)

                block = Entities.create('blockWall')
                block:setPosition(roomWidth - wallWidth, 0)

                --Text
                storage.font = Fonts.getFont('default')

                --Sounds
                player = Sounds.createPlayer()
                player:setStatic(true)
                player:setSound(Sounds.getSound('theme'))
                player:setLooping(true);
                player:play();

                --Enemies
                enemyStates = {
                    live = function()
                        local x, y = entity:getPosition()
                        local roomWidth = Window.getRoom()
                        local width, _ = entity:getSprite():getSize()
                        local offset, _ = entity:getSprite():getOffset()

                        local speed, speedIndex;
                        if (gameState == 'prepare') then
                            if x < -(width - offset) or x > roomWidth + offset then
                                Entities.destroy(entity)

                                floors[storage.floor].count = floors[storage.floor].count - 1
                                enemyCount = enemyCount - 1
                            end

                            speed = prepareEnemySpeed
                            speedIndex = prepareEnemySpeedIndex
                        else

                            Entities.foreachType('enemy', function()
                                if (entity ~= that and storage.state == 'live' and storage.floor == that:getStorage().floor) then
                                    local x1, _ = entity:getPosition()
                                    local x2, _ = that:getPosition()

                                    if (math.abs(x1 - x2) <= 16) then
                                        if (x1 < x2) then
                                            storage.speedScale = -1
                                            if (storage.offset) then
                                                storage.offset = 0
                                                entity:setIndex(0 + (entity:getIndex() % 2))
                                            end
                                            that:getStorage().speedScale = 1
                                            that:getStorage().offset = 2
                                            that:setIndex(2 + (that:getIndex() % 2))
                                        else
                                            storage.speedScale = 1
                                            if (storage.offset) then
                                                storage.offset = 2
                                                entity:setIndex(2 + (entity:getIndex() % 2))
                                            end
                                            that:getStorage().speedScale = -1
                                            that:getStorage().offset = 0
                                            that:setIndex(0 + (that:getIndex() % 2))
                                        end

                                    end
                                end
                            end)

                            if x < wallWidth + offset then
                                storage.speedScale = 1
                                storage.offset = 2
                                entity:setIndex(storage.offset + (entity:getIndex() % 2))
                            elseif x > roomWidth - (wallWidth + (width - offset)) then
                                storage.speedScale = -1
                                storage.offset = 0
                                entity:setIndex(storage.offset + (entity:getIndex() % 2))
                            end

                            speed = storage.speed
                            speedIndex = storage.speedIndex
                        end

                        x = x + storage.speedScale * speed * Window.getDelta()
                        entity:setPosition(x, y)

                        storage.indexAgo = storage.indexAgo + Window.getDelta()
                        if storage.indexAgo >= speedIndex then
                            repeat
                                storage.indexAgo = storage.indexAgo - speedIndex
                            until storage.indexAgo < speedIndex
                            entity:setIndex(storage.offset + ((entity:getIndex() + 1) % 2))
                        end
                    end,
                    dying = function()
                        local x, y = entity:getPosition()
                        local roomWidth, roomHeight = Window.getRoom()
                        local width, _ = entity:getSprite():getSize()
                        local offsetX, offsetY = entity:getSprite():getOffset()

                        if x < (offsetX - width) or x > (roomWidth + offsetX) or
                                y > (roomHeight + offsetY) then
                            Entities.destroy(entity)
                        else
                            entity:setPosition(x + storage.speedX * Window.getDelta(), y + storage.speedY * Window.getDelta())
                            storage.speedY = storage.speedY + gravity * Window.getDelta()
                        end
                    end
                }

                --Global vars
                bulletSpeed = Speed
                bulletHitDistance = 10
                gravity = 300

                timeRemaining = Time
                enemyCost = {
                    goomba = 2,
                    koopa_troopa = 1
                }

                gameState = 'start'

                prepareEnemySpeed = 200
                prepareEnemySpeedIndex = 0.1

                storage.enemyCount = CountTarget
                enemyCount = 0
                enemyLeft = CountTarget
                storage.playTime = Time
                storage.leftTime = Time

                storage.bulletDistance = 10

                storage.floorCount = 6

                storage.blockHeight = 16
                storage.startY = 24

                local timeBuffer = 10
                storage.inverseEnemySpawn = (Time - math.min(timeBuffer, Time)) / CountTarget
                storage.enemySpawnAgo = 0

                floors = {}
                floors.max = 1
                floors.updateMax = function()
                    floors.max = 0
                    if #floors > 0 then
                        floors.max = floors[1].count
                    else
                        return
                    end
                    local flag = true
                    for i=2, #floors do
                        if floors[i].count > floors.max then
                            floors.max = floors[i].count
                            flag = false
                        elseif floors[i].count ~= floors.max then
                            flag = false
                        end
                    end
                    if flag then
                        floors.max = floors.max + 1
                    end
                end
                local floorY = storage.startY
                for i = 1, storage.floorCount do
                    block = Entities.create('floor')
                    block:setPosition(wallWidth, floorY)
                    floors[i] = {}
                    floors[i].y = floorY
                    floors[i].count = 0
                    floorY = floorY + storage.blockHeight * 2
                end

                local rest = Entities.create('rest')
                rest:setPosition(roomWidth / 2, wallHeight)

                local gun = Entities.create('gun')
                gun:setPosition(roomWidth / 2, wallHeight - 20)
            end,
            onStep = function()
                -- Service
                if Input.isKeyTyped(Input.KEY_ESCAPE) then
                    Window.closeQuery()
                end
                if Input.isKeyTyped(Input.KEY_F4) then
                    if Window.getWindowMode() == Window.MODE_WINDOWED then
                        Window.setWindowMode(Window.MODE_FULLSCREEN)
                    else
                        Window.setWindowMode(Window.MODE_WINDOWED)
                    end
                end

                -- Game
                ({
                    start = function()
                        if (Input.isAnyKeyTouched() or Input.isAnyMouseButtonTouched()) then
                            gameState = 'prepare'

                            if (enemyCount > 0) then
                                player:setSound(Sounds.getSound('prepare'))
                                player:setLooping(true)
                                player:play()
                            end
                        end

                        local fontWidth, fontHeight = storage.font:getSize()
                        local roomWidth, roomHeight = Window.getRoom()
                        local margin = fontHeight / 4

                        storage.font:draw("MARIO RIFLE RANGE", roomWidth / 2, roomHeight / 2 - (fontHeight + 2 * margin), bit.bor(Fonts.DOWN, Fonts.H_CENTER), -5)

                        storage.font:draw("TO START GAME", roomWidth / 2, roomHeight / 2 + margin, bit.bor(Fonts.UP, Fonts.H_CENTER), -5)
                        storage.font:draw("PRESS ANY KEY", roomWidth / 2, roomHeight / 2 + (fontHeight + 2 * margin), bit.bor(Fonts.UP, Fonts.H_CENTER), -5)

                        if storage.enemySpawnAgo < storage.inverseEnemySpawn then
                            storage.enemySpawnAgo = storage.enemySpawnAgo + Window.getDelta()
                        end

                        if storage.enemySpawnAgo >= storage.inverseEnemySpawn and enemyCount < storage.enemyCount then
                            for i=1, storage.floorCount do
                                if floors[i].count < floors.max then
                                    local enemy
                                    if math.random() < 0.5 then
                                        enemy = Entities.create('goomba')
                                    else
                                        enemy = Entities.create('koopa_troopa')
                                    end
                                    local offset, _ = enemy:getSprite():getOffset()
                                    local xSpawn
                                    if math.random() < 0.5 then
                                        local width, _ = enemy:getSprite():getSize()
                                        xSpawn = wallWidth - (width - offset)
                                    else
                                        local roomWidth, _ = Window.getRoom()
                                        xSpawn = roomWidth - wallWidth + offset
                                    end
                                    enemy:setPosition(xSpawn, floors[i].y)
                                    enemy:getStorage().floor = i

                                    floors[i].count = floors[i].count + 1
                                    enemyCount = enemyCount + 1
                                    storage.enemySpawnAgo = 0

                                    floors.updateMax()
                                    break
                                end
                            end
                        end
                    end,
                    prepare = function()
                        if (enemyCount == 0) then
                            floors.updateMax()
                            storage.enemySpawnAgo = 0

                            enemyLeft = storage.enemyCount
                            storage.leftTime = storage.playTime

                            gameState = 'game'

                            player:setSound(Sounds.getSound('game'))
                            player:setLooping(true)
                            player:play()
                        end
                    end,
                    game = function()
                        storage.leftTime = storage.leftTime - Window.getDelta()
                        if (storage.leftTime <= 0) then
                            gameState = "over"

                            player:setSound(Sounds.getSound('over'))
                            player:setLooping(false)
                            player:play()
                        end

                        local roomWidth, _ = Window.getRoom()
                        local _, height = storage.font:getSize();
                        local margin, alignmentLeft, alignmentTime, depth = height / 4, bit.bor(Fonts.LEFT, Fonts.UP), bit.bor(Fonts.RIGHT, Fonts.UP), -5

                        storage.font:draw('LEFT', margin, margin, alignmentLeft, depth)
                        storage.font:draw(string.format('%.3d', enemyLeft), margin, height + 2 * margin, alignmentLeft, depth)

                        storage.font:draw('TIME', roomWidth - margin, margin, alignmentTime, depth)
                        storage.font:draw(string.format('%.3d', math.ceil(storage.leftTime)), roomWidth - margin, height + 2 * margin, alignmentTime, depth)

                        if storage.enemySpawnAgo < storage.inverseEnemySpawn then
                            storage.enemySpawnAgo = storage.enemySpawnAgo + Window.getDelta()
                        end

                        if storage.enemySpawnAgo >= storage.inverseEnemySpawn and enemyCount < enemyLeft then
                            for i=1, storage.floorCount do
                                if floors[i].count < floors.max then
                                    local enemy
                                    if math.random() < 0.5 then
                                        enemy = Entities.create('goomba')
                                    else
                                        enemy = Entities.create('koopa_troopa')
                                    end
                                    local offset, _ = enemy:getSprite():getOffset()
                                    local xSpawn
                                    if math.random() < 0.5 then
                                        local width, _ = enemy:getSprite():getSize()
                                        xSpawn = wallWidth - (width - offset)
                                    else
                                        local roomWidth, _ = Window.getRoom()
                                        xSpawn = roomWidth - wallWidth + offset
                                    end
                                    enemy:setPosition(xSpawn, floors[i].y)
                                    enemy:getStorage().floor = i

                                    floors[i].count = floors[i].count + 1
                                    enemyCount = enemyCount + 1
                                    storage.enemySpawnAgo = 0

                                    floors.updateMax()
                                    break
                                end
                            end
                        end
                    end,
                    win = function()
                        if (Input.isKeyTouched(Input.KEY_Y)) then
                            gameState = 'prepare'
                        elseif (Input.isKeyTouched(Input.KEY_N)) then
                            Window.closeQuery()
                        end

                        local fontWidth, fontHeight = storage.font:getSize()
                        local roomWidth, roomHeight = Window.getRoom()
                        local margin = fontHeight / 4

                        storage.font:draw("YOU WIN!", roomWidth / 2, roomHeight / 2 - (fontHeight + 2 * margin), bit.bor(Fonts.DOWN, Fonts.H_CENTER), -5)

                        storage.font:draw("REPEAT?", roomWidth / 2, roomHeight / 2 + (margin), bit.bor(Fonts.UP, Fonts.H_CENTER), -5)
                        storage.font:draw("Y / N", roomWidth / 2, roomHeight / 2 + (fontHeight + 2 * margin), bit.bor(Fonts.UP, Fonts.H_CENTER), -5)
                    end,
                    over = function()
                        if (Input.isKeyTouched(Input.KEY_Y)) then
                            gameState = 'prepare'

                            if (enemyCount > 0) then
                                player:setSound(Sounds.getSound('prepare'))
                                player:setLooping(true)
                                player:play()
                            end
                        elseif (Input.isKeyTouched(Input.KEY_N)) then
                            Window.closeQuery()
                        end

                        local fontWidth, fontHeight = storage.font:getSize()
                        local roomWidth, roomHeight = Window.getRoom()
                        local margin = fontHeight / 4

                        storage.font:draw("GAME OVER", roomWidth / 2, roomHeight / 2 - (1.5 * fontHeight + 2 * margin), bit.bor(Fonts.DOWN, Fonts.H_CENTER), -5)
                        storage.font:draw(string.format("ENEMIES LEFT: %d", enemyLeft), roomWidth / 2, roomHeight / 2 - (0.5 * fontHeight + margin), bit.bor(Fonts.DOWN, Fonts.H_CENTER), -5)

                        storage.font:draw("REPEAT?", roomWidth / 2, roomHeight / 2 + (0.5 * fontHeight + margin), bit.bor(Fonts.UP, Fonts.H_CENTER), -5)
                        storage.font:draw("Y / N", roomWidth / 2, roomHeight / 2 + (1.5 * fontHeight + 2 * margin), bit.bor(Fonts.UP, Fonts.H_CENTER), -5)
                    end
                })[gameState]()

            end
        },
        {
            name = 'gun',
            sprite = 'gun',
            depth = 0,
            onCreate = function()
                local bulletsInSecond = 3
                storage.inverseSpeed = 1 / bulletsInSecond
                storage.lastBulletAgo = 0
            end,
            onStep = function()
                if storage.lastBulletAgo < storage.inverseSpeed then
                    storage.lastBulletAgo = storage.lastBulletAgo + Window.getDelta()
                end

                local mouseX, mouseY = Input.getMousePos()
                if mouseX and mouseY then
                    local x, y = entity:getPosition()
                    local relX, relY = mouseX - x, mouseY - y
                    local angle = math.deg(math.acos(relX/((relX * relX + relY * relY)^0.5)))
                    if mouseY > y then
                        angle = 360 - angle
                    end

                    local angleBorder = 30
                    if (angle > 180 - angleBorder and angle < 270) then angle = 180 - angleBorder end
                    if (angle > 270 or angle < angleBorder) then angle = angleBorder end
                    entity:setRotation(angle)

                    if gameState == 'game' and
                            Input.isMouseButtonPressed(Input.MOUSE_BUTTON_LEFT) and
                            storage.lastBulletAgo >= storage.inverseSpeed then
                        storage.lastBulletAgo = 0

                        local bullet = Entities.create('bullet')
                        bullet:setPosition(entity:getPosition())
                        bullet:setRotation(angle)

                        local player = Sounds.createPlayer()
                        player:setSound(Sounds.getSound('bang'))
                        player:play()

                        local smoke = Particles.create('smoke')
                        smoke:setPosition(entity:getPosition())
                        smoke:setAngle(entity:getRotation())
                    end
                end
            end
        },
        {
            name = 'bullet',
            sprite = 'bullet',
            type = 'bullet',
            depth = 2,
            onCreate = function()
                storage.wallOffset = 20
                storage.checkAngle = function()
                    local angle = Common.cropDeg(entity:getRotation() + 90)
                    if angle < 180 then
                        entity:setIndex(1)
                    else
                        entity:setIndex(0)
                    end
                end
            end,
            onStep = function()
                local step = bulletSpeed * Window.getDelta()
                local angle = math.rad(entity:getRotation())

                local roomWidth, roomHeight = Window.getRoom()

                local x, y = entity:getPosition()
                local xDelta, yDelta = math.cos(angle) * step, -math.sin(angle) * step
                local xNew, yNew = x + xDelta, y + yDelta

                local farOffset = roomWidth - storage.wallOffset

                local soundFlag = false
                while (xNew < storage.wallOffset) or (xNew > farOffset) do
                    entity:setRotation(180 - entity:getRotation())
                    soundFlag = true

                    if xNew < storage.wallOffset  then
                        xNew = 2 * storage.wallOffset - xNew
                        x = storage.wallOffset
                    else
                        xNew = 2 * farOffset - xNew
                        x = farOffset
                    end
                end
                if (soundFlag) then
                    local blockHit = Sounds.createPlayer()
                    blockHit:setSound(Sounds.getSound('block'))
                    blockHit:play()
                end

                entity:setPosition(xNew, yNew)

                storage.checkAngle()

                if y < -16 then
                    Entities.destroy(entity)
                end

                Entities.foreachType('enemy', function()
                    if (storage.state ~= 'live') then
                        return false
                    end

                    local x, y = entity:getPosition()
                    y = y - storage.centerOffset
                    local thatX, thatY = that:getPosition()
                    local difX, difY = x - thatX, y - thatY
                    local distance = bulletHitDistance
                    if difX * difX + difY * difY <= distance * distance then
                        storage.state = 'dying'
                        entity:setIndex(entity:getSprite():getSlides() - 1)
                        entity:setDepth(-6)

                        Entities.destroy(that)

                        floors[storage.floor].count = floors[storage.floor].count - 1
                        enemyCount = enemyCount - 1

                        local hit = Sounds.createPlayer()
                        hit:setSound(Sounds.getSound('hit'))
                        hit:play()

                        local smoke = Particles.create('hit')
                        local smokeX, smokeY = entity:getPosition()
                        smokeY = smokeY - storage.centerOffset
                        smoke:setPosition(smokeX, smokeY)

                        if (gameState == 'game') then
                            enemyLeft = enemyLeft - 1
                            if (enemyLeft <= 0) then
                                gameState = 'win'

                                player:setSound(Sounds.getSound('win'))
                                player:setLooping(false)
                                player:play()
                            end
                        end

                        floors.updateMax()
                        return true;
                    end
                end)
            end
        },
        {
            name = 'rest',
            sprite = 'rest',
            depth = -2
        },
        {
            name = 'blockWall',
            sprite = 'blockWall',
            depth = -4
        },
        {
            name = 'floor',
            sprite = 'floor',
            depth = 4
        },
        {
            name = 'goomba',
            sprite = 'goomba',
            type = 'enemy',
            depth = 3,
            onCreate = function()
                --live
                storage.centerOffset = 8
                storage.floor = 1
                local speedMin, speedMax = 40, 60
                storage.speed = math.random(speedMin, speedMax)
                storage.speedIndex = 0.5
                storage.speedScale = 1
                storage.indexAgo = storage.speedIndex
                storage.offset = 0

                --dying
                local minSpeedX, maxSpeedX = 20, 40
                local speedX = math.random(minSpeedX, maxSpeedX)
                if math.random() < 0.5 then
                    storage.speedX = speedX
                else
                    storage.speedX = -speedX
                end

                local minSpeedY, maxSpeedY = 50, 70
                storage.speedY = -math.random(minSpeedY, maxSpeedY)

                --states
                storage.state = 'live'
            end,
            onStep = function()
                enemyStates[storage.state]()
            end
        },
        {
            name = 'koopa_troopa',
            sprite = 'koopa_troopa',
            type = 'enemy',
            depth = 3,
            onCreate = function()
                --live
                storage.centerOffset = 12
                storage.floor = 1
                local speedMin, speedMax = 20, 40
                storage.speed = math.random(speedMin, speedMax)
                storage.speedIndex = 0.5
                storage.speedScale = 1
                storage.indexAgo = storage.speedIndex
                storage.offset = 0

                --dying
                local minSpeedX, maxSpeedX = 20, 40
                local speedX = math.random(minSpeedX, maxSpeedX)
                if math.random() < 0.5 then
                    storage.speedX = speedX
                else
                    storage.speedX = -speedX
                end

                local minSpeedY, maxSpeedY = 50, 70
                storage.speedY = -math.random(minSpeedY, maxSpeedY)

                --states
                storage.state = 'live'
            end,
            onStep = function()
                enemyStates[storage.state]()
            end
        }, {
            name = 'cloud',
            sprite = 'cloud',
            depth = 6,
            onCreate = function()
                local x, y

                local minSpeed, maxSpeed = 10, 30
                local speed = math.random(minSpeed, maxSpeed)
                local offset, _ = entity:getSprite():getOffset()
                if math.random() < 0.5 then
                    storage.speed = speed
                    local width, _ = entity:getSprite():getSize()
                    x = offset - width
                else
                    storage.speed = -speed
                    local widthRoom, _ = Window.getRoom()
                    x = widthRoom + offset
                end

                local minY, maxY = 0, 176
                y = math.random(minY, maxY)
                entity:setPosition(x, y)
            end,
            onStep = function()
                local x, y = entity:getPosition()
                x = x + storage.speed * Window.getDelta()
                entity:setPosition(x, y)

                local widthRoom, _ = Window.getRoom();
                local width, _ = entity:getSprite():getSize();
                local offset, _ = entity:getSprite():getOffset();

                if x < (offset - width) or x > (widthRoom + offset) then
                    Entities.destroy(entity)
                    Entities.create('cloud')
                end
            end
        }
    },
    room = {
        {
            prototype = 'manager'
        }
    }
};