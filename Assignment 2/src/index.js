import * as THREE from 'three'
import { GLTFLoader } from 'three/examples/jsm/loaders/GLTFLoader.js'
const carcolors = [0xa52523, 0xffff00, 0x0000ff, 0xffffff]
const gltfLoader = new GLTFLoader();
function Car(index) {
    let car = new THREE.Group()
    const Backwheel = new THREE.Mesh(
        new THREE.BoxGeometry(12, 33, 12),
        new THREE.MeshLambertMaterial({ color: 0x333333 })
    )
    Backwheel.position.z = 6;
    Backwheel.position.x = -18;
    car.add(Backwheel)
    const Frontwheel = new THREE.Mesh(
        new THREE.BoxGeometry(12, 33, 12),
        new THREE.MeshLambertMaterial({ color: 0x333333 })
    )
    Frontwheel.position.z = 6;
    Frontwheel.position.x = 18;
    car.add(Frontwheel)
    const carFront = carFrontTexture()
    carFront.center = new THREE.Vector2(0.5, 0.5)
    carFront.rotation = Math.PI / 2
    const carBack = carFrontTexture()
    carBack.center = new THREE.Vector2(0.5, 0.5)
    carBack.rotation = -Math.PI / 2
    const carLeft = carSideTexture()
    carLeft.flipY = false
    const carRight = carSideTexture()
    const engine = new THREE.Mesh(
        new THREE.BoxGeometry(60, 30, 15),
        new THREE.MeshLambertMaterial({ color: carcolors[index] })
    )
    engine.position.z = 12;
    car.add(engine)
    const top = new THREE.Mesh(
        new THREE.BoxGeometry(33, 24, 12),
        [
            new THREE.MeshLambertMaterial({ map: carFront }),
            new THREE.MeshLambertMaterial({ map: carBack }),
            new THREE.MeshLambertMaterial({ map: carLeft }),
            new THREE.MeshLambertMaterial({ map: carRight }),
            new THREE.MeshLambertMaterial({ color: 0xffffff }),
            new THREE.MeshLambertMaterial({ color: 0xffffff })
        ]
    )
    top.position.x = -6;
    top.position.z = 25.5;
    car.add(top)
    return car
}
function carFrontTexture() {
    const canvas = document.createElement("canvas")
    canvas.width = 64;
    canvas.height = 32;
    const context = canvas.getContext("2d")
    context.fillStyle = "#ffffff"
    context.fillRect(0, 0, 64, 32)
    context.fillStyle = "#666666"
    context.fillRect(8, 8, 48, 24)
    return new THREE.CanvasTexture(canvas)
}

function carSideTexture() {
    const canvas = document.createElement("canvas")
    canvas.width = 128;
    canvas.height = 32;
    const context = canvas.getContext("2d")
    context.fillStyle = "#ffffff"
    context.fillRect(0, 0, 128, 32)
    context.fillStyle = "#666666"
    context.fillRect(8, 8, 38, 24)
    context.fillRect(54, 8, 60, 24)
    return new THREE.CanvasTexture(canvas)
}
function getLineMarkings(mapWidth, mapHeight) {
    const canvas = document.createElement("canvas")
    canvas.width = mapWidth
    canvas.height = mapHeight
    const context = canvas.getContext("2d")
    context.fillStyle = "#2F4F4F"
    context.fillRect(0, 0, mapWidth, mapHeight)
    context.lineWidth = 2
    context.strokeStyle = "#E0FFFF"
    context.setLineDash([8, 14])
    context.beginPath()
    context.arc(
        mapWidth / 2,
        mapHeight / 2,
        trackRadius,
        0,
        2 * Math.PI
    )
    context.stroke()
    return new THREE.CanvasTexture(canvas)
}

function getMidIsland() {
    const islandMid = new THREE.Shape()
    islandMid.absarc(
        0, 0, innerRadius, 0, Math.PI, true
    )
    islandMid.absarc(
        0, 0, innerRadius, 0, 2 * Math.PI, true
    )
    return islandMid
}
function getOutIsland(mapWidth, mapHeight) {
    const field = new THREE.Shape()
    field.moveTo(-mapWidth / 2, -mapHeight / 2)
    field.lineTo(0, -mapHeight / 2)
    field.absarc(
        0, 0, outerRadius, -Math.PI / 2, (3 / 2) * Math.PI, true
    )
    field.lineTo(0, -mapHeight / 2)
    field.lineTo(mapWidth / 2, -mapHeight / 2)
    field.lineTo(mapWidth / 2, mapHeight / 2)
    field.lineTo(-mapWidth / 2, mapHeight / 2)
    return field
}
function renderMap(mapWidth, mapHeight) {
    const lineMarkings = getLineMarkings(mapWidth, mapHeight)
    const planeGeometry = new THREE.PlaneGeometry(mapWidth, mapHeight)
    const planeMaterial = new THREE.MeshLambertMaterial(
        { map: lineMarkings }
    )
    const plane = new THREE.Mesh(planeGeometry, planeMaterial)
    scene.add(plane)

    const islandMid = getMidIsland()
    const islandOut = getOutIsland(1000000, 1000000)

    const fieldGeometry = new THREE.ExtrudeGeometry([
        islandMid, islandOut
    ], { depth: 6, bevelEnabled: false })

    const fieldMesh = new THREE.Mesh(fieldGeometry, [
        new THREE.MeshLambertMaterial({ color: 0x67c240 }),
        new THREE.MeshLambertMaterial({ color: 0x23311c })
    ])
    scene.add(fieldMesh)
}
function getPlayerSpeed(speed_veh) {
    if (accelerate) {
        if ((outtrack == true) && speed_veh > 0.1) {
            return speed_veh - 0.01
        }
        if ((outtrack == true) && speed_veh < -0.1) {
            return speed_veh + 0.01
        }
        else if ((outtrack == true)) {
            return 0.1
        }
        if ((right != 0) && speed_veh > 0.001) {
            return speed_veh - 0.001
        }
        if ((right != 0) && speed_veh < -0.001) {
            return speed_veh + 0.001
        }
        else if ((right != 0)) {
            return 0
        }
        if (speed_veh < 1.1) {
            speed_veh += 0.02
        }
        else speed_veh = 1.1
    }
    else if (decelerate) {
        if ((outtrack == true) && speed_veh > 0.1) {
            return speed_veh - 0.01
        }
        if ((outtrack == true) && speed_veh < -0.1) {
            return speed_veh + 0.01
        }
        else if ((outtrack == true)) {
            return -0.1
        }
        if ((right != 0) && speed_veh > 0.001) {
            return speed_veh - 0.001
        }
        if ((right != 0) && speed_veh < -0.001) {
            return speed_veh + 0.001
        }
        else if ((right != 0)) {
            return 0
        }
        if (speed_veh > -1.1) {
            speed_veh -= 0.02
        }
        else speed_veh = -1.1
    }
    else {
        if (speed_veh > 0.0005) {
            if(speed_veh > 0.01 && outtrack==true) speed_veh-=0.01
            else speed_veh -= 0.0005
        }
        else if (speed_veh < -0.0005) {
            if(speed_veh < -0.01 && outtrack==true) speed_veh+=0.01
            else speed_veh += 0.0005
        }
        else speed_veh = 0
    }
    return speed_veh
}
function MovePlayer(timeDelta) {
    if (flag == true) {
        window.fuelcan.rotation.y += 0.03

        speed = getPlayerSpeed(speed)
        const position = new THREE.Vector3(playerCar.position.x, playerCar.position.y, playerCar.position.z)
        playerCar.translateX(timeDelta * speed);
        fuel -= timeDelta * speed / mileage
        if (Math.ceil(fuel) < 0) {
            end = true;
            const text4 = addText("Game Over --->  Player Out of Fuel", 50, 10, 35)
            const text5 = addText("LeaderBoard:", 50, 20, 35)
            let arr = [{ score: score, player: "You" }, { score: score2, player: "Player1" }, { score: score3, player: "Player3" }, { score: score4, player: "Player4" }]
            arr.sort(function (a, b) { return b.score - a.score })
            for (let i = 0; i < 4; i++) {
                addText(arr[i].player + " : " + arr[i].score, 30, 30 + i * 10, 35);
            }
        }
        if (speed != 0) {
            playerCar.rotation.z += right
        }
        if (position.y <= 0 && playerCar.position.y > 0) {
            score = score + 1
        }
        if (toggle == true) {
            if (speed >= 0) {
                camera.position.x = playerCar.position.x + (position.x - playerCar.position.x) * 40
                camera.position.y = playerCar.position.y + (position.y - playerCar.position.y) * 40
            }
            else if (speed < 0) {
                camera.position.x = playerCar.position.x + (-position.x + playerCar.position.x) * 40
                camera.position.y = playerCar.position.y + (-position.y + playerCar.position.y) * 40
            }
            camera.position.z = 200;
            camera.lookAt(playerCar.position.x, playerCar.position.y, playerCar.position.z)
        }
        else {
            if (speed >= 0) {
                camera.position.x = playerCar.position.x - (position.x - playerCar.position.x) * 3
                camera.position.y = playerCar.position.y - (position.y - playerCar.position.y) * 3
            }
            else if (speed < 0) {
                camera.position.x = playerCar.position.x - (-position.x + playerCar.position.x) * 3
                camera.position.y = playerCar.position.y - (-position.y + playerCar.position.y) * 3
            }
            camera.position.z = 80;
            if (speed > 0) {
                camera.lookAt(playerCar.position.x - (position.x - playerCar.position.x) * 20,
                    playerCar.position.y - (position.y - playerCar.position.y) * 30, playerCar.position.z)
            }
            else if (speed < 0) {
                camera.lookAt(playerCar.position.x + (position.x - playerCar.position.x) * 20,
                    playerCar.position.y + (position.y - playerCar.position.y) * 30, playerCar.position.z)
            }

        }
    }
}
function moveOtherVehicles(timeDelta) {
    if (flag == true) {
        let x = (Math.random() / 1000) + 0.0005
        angle2 -= timeDelta * x
        if (angle2 < 2 * Math.PI) {
            angle2 += 2 * Math.PI
        }
        let position = [...playerCar2.position]
        let playerX = Math.cos(angle2) * (trackRadius - 150 + right2)
        let playerY = Math.sin(angle2) * (trackRadius - 150 + right2)
        if (trackRadius - 150 + right2 < trackRadius - 20 && flagright1 == 1) {
            right2 += 1
        }
        else if (flagright1 == 1) {
            flagright1 = -1;
        }
        else if (trackRadius - 150 + right2 > innerRadius + 20 && flagright1 == -1) {
            right2 -= 1
        }
        else {
            flagright1 = 1;
        }
        playerCar2.position.x = playerX
        playerCar2.position.y = playerY
        playerCar2.rotation.z = angle2 - Math.PI / 2
        if (position[1] <= 0 && playerCar2.position.y > 0) {
            score2 = score2 + 1
        }

        angle3 -= timeDelta * ((Math.random() / 1000) + 0.0005)
        position = [...playerCar3.position]
        if (angle3 < 2 * Math.PI) {
            angle3 += 2 * Math.PI
        }
        playerX = Math.cos(angle3) * (trackRadius + 150 + right3)
        playerY = Math.sin(angle3) * (trackRadius + 150 + right3)
        if (trackRadius + 150 + right3 < outerRadius - 20 && flagright2 == 1) {
            right3 += 1
        }
        else if (flagright2 == 1) {
            flagright2 = -1;
        }
        else if (trackRadius + 150 + right3 > trackRadius + 20 && flagright2 == -1) {
            right3 -= 1
        }
        else {
            flagright2 = 1;
        }
        playerCar3.position.x = playerX
        playerCar3.position.y = playerY
        playerCar3.rotation.z = angle3 - Math.PI / 2
        if (position[1] <= 0 && playerCar3.position.y > 0) {
            score3 = score3 + 1
        }

        angle4 -= timeDelta * x
        position = [...playerCar4.position]
        if (angle4 < 2 * Math.PI) {
            angle4 += 2 * Math.PI
        }
        playerX = Math.cos(angle4) * (trackRadius - 150 + right4)
        playerY = Math.sin(angle4) * (trackRadius - 150 + right4)
        if (trackRadius - 150 + right4 < trackRadius - 20 && flagright3 == 1) {
            right4 += 1
        }
        else if (flagright3 == 1) {
            flagright3 = -1;
        }
        else if (trackRadius - 150 + right4 > innerRadius + 20 && flagright3 == -1) {
            right4 -= 1
        }
        else {
            flagright3 = 1;
        }
        playerCar4.position.x = playerX
        playerCar4.position.y = playerY
        playerCar4.rotation.z = angle4 - Math.PI / 2
        if (position[1] <= 0 && playerCar4.position.y > 0) {
            score4 = score4 + 1
        }
    }
}

function startGame() {
    flag = true
}
let flag = false
let end = false
let toggle = true
let right = 0
let right2 = 0, right3 = 0, right4 = 0
let flagright1 = Math.random() > 0.5 ? 1 : -1, flagright2 = Math.random() > 0.5 ? 1 : -1, flagright3 = Math.random() > 0.5 ? 1 : -1
let camAngle = Math.PI + Math.PI / 3
let angle1 = Math.PI + Math.PI / 5
let angle2 = Math.PI + Math.PI / 18
let score2 = -1, score3 = -1, score4 = -1
let angle3 = Math.PI + Math.PI / 18
let angle4 = Math.PI + Math.PI / 5
let lastTimestamp
const trackRadius = 1000
const trackWidth = 400
const innerRadius = trackRadius - trackWidth
const outerRadius = trackRadius + trackWidth
const scene = new THREE.Scene();
function addText(text, size, x, y) {
    var text2 = document.createElement('div');
    text2.style.position = 'absolute';
    text2.style.width = 100;
    text2.style.height = 100;
    text2.style.fontSize = size + "px"
    text2.style.backgroundColor = "aqua";
    text2.innerHTML = text;
    text2.style.top = x + '%';
    text2.style.left = y + '%';
    document.body.appendChild(text2);
    return text2
}
function Calculatehits() {
    let d1 = new Date()
    let now = d1.getTime()
    var bbox = new THREE.Box3().setFromObject(playerCar);
    let center = new THREE.Vector3()
    bbox.getCenter(center)
    let angle = Math.atan(center.y / center.x)
    if (center.x < 0) {
        angle += Math.PI
    }
    if (angle < 0) {
        angle += 2 * Math.PI
    }

    var bfuel = new THREE.Box3().setFromObject(window.fuelcan);
    center = new THREE.Vector3()
    bfuel.getCenter(center)
    let angle_fuel = Math.atan(center.y / center.x)
    if (center.x < 0) {
        angle_fuel += Math.PI
    }
    if (angle_fuel < 0) {
        angle_fuel += 2 * Math.PI
    }

    let diff = angle - angle_fuel;
    if (diff < 0) diff += 2 * Math.PI
    document.getElementById("next_fuel").innerHTML = Math.round(trackRadius * (diff)/100)
    if (now - lasthit > 300) {
        if (hitDetection(playerCar, playerCar2) ||
            hitDetection(playerCar, playerCar3) ||
            hitDetection(playerCar, playerCar4)) {
            health -= 10
            speed = 0;
            lasthit = now;
            if (health <= 0) {
                end = true;
                document.getElementById("health").innerHTML = 0;
                const text4 = addText("Game Over --->  LeaderBoard:", 50, 20, 35)
                let arr = [{ score: score, player: "You" }, { score: score2, player: "Player1" }, { score: score3, player: "Player3" }, { score: score4, player: "Player4" }]
                arr.sort(function (a, b) { return b.score - a.score })
                for (let i = 0; i < 4; i++) {
                    addText(arr[i].player + " : " + arr[i].score, 30, 30 + i * 10, 35);
                }
            }
        }
        if (hitDetection(playerCar, window.fuelcan)) {
            fuel = 100;
            let angle = Math.random() * 2 * Math.PI
            window.fuelcan.position.set(trackRadius * Math.cos(angle), trackRadius * Math.sin(angle), 10)
            lasthit = now
        }

        if (Math.pow(bbox.min.x, 2) + Math.pow(bbox.min.y, 2) > Math.pow((outerRadius + 10), 2) ||
            Math.pow(bbox.min.x, 2) + Math.pow(bbox.max.y, 2) > Math.pow((outerRadius + 10), 2) ||
            Math.pow(bbox.max.x, 2) + Math.pow(bbox.min.y, 2) > Math.pow((outerRadius + 10), 2) ||
            Math.pow(bbox.max.x, 2) + Math.pow(bbox.max.y, 2) > Math.pow((outerRadius + 10), 2)) {
            health -= 2
            if (health <= 0) {
                end = true;
                document.getElementById("health").innerHTML = 0;
                const text4 = addText("Game Over --->  LeaderBoard:", 50, 20, 35)
                let arr = [{ score: score, player: "You" }, { score: score2, player: "Player1" }, { score: score3, player: "Player3" }, { score: score4, player: "Player4" }]
                arr.sort(function (a, b) { return b.score - a.score })
                for (let i = 0; i < 4; i++) {
                    addText(arr[i].player + " : " + arr[i].score, 30, 30 + i * 10, 35);
                }
            }
            lasthit = now;
            outtrack = true
        }
        else {
            outtrack = false;

            if (Math.pow(bbox.min.x, 2) + Math.pow(bbox.min.y, 2) < Math.pow((innerRadius - 10), 2) ||
                Math.pow(bbox.min.x, 2) + Math.pow(bbox.max.y, 2) < Math.pow((innerRadius - 10), 2) ||
                Math.pow(bbox.max.x, 2) + Math.pow(bbox.min.y, 2) < Math.pow((innerRadius - 10), 2) ||
                Math.pow(bbox.max.x, 2) + Math.pow(bbox.max.y, 2) < Math.pow((innerRadius - 10), 2)) {
                health -= 2
                if (health <= 0) {
                    end = true;
                    document.getElementById("health").innerHTML = 0;
                    const text4 = addText("Game Over --->  LeaderBoard:", 50, 20, 35)
                    let arr = [{ score: score, player: "You" }, { score: score2, player: "Player1" }, { score: score3, player: "Player3" }, { score: score4, player: "Player4" }]
                    arr.sort(function (a, b) { return b.score - a.score })
                    for (let i = 0; i < 4; i++) {
                        addText(arr[i].player + " : " + arr[i].score, 30, 30 + i * 10, 35);
                    }
                }
                lasthit = now;
                outtrack = true
            }
            else {
                outtrack = false;
            }
        }
    }
}
function hitDetection(object1, object2) {
    var bbox = new THREE.Box3().setFromObject(object1);
    var bbox2 = new THREE.Box3().setFromObject(object2);
    let collidex = false, collidey = false
    let low1 = bbox.min.x, low2 = bbox2.min.x
    let high1 = bbox.max.x, high2 = bbox2.max.x
    if ((low1 < high2 && low1 > low2) || (high1 < high2 && high1 > low2) || (high1 > high2 && low1 < low2)) {
        collidex = true;
    }
    low1 = bbox.min.y, low2 = bbox2.min.y
    high1 = bbox.max.y, high2 = bbox2.max.y
    if ((low1 < high2 && low1 > low2) || (high1 < high2 && high1 > low2) || (high1 > high2 && low1 < low2)) {
        collidey = true;
    }
    if (collidex && collidey) {
        return true
    }
    else {
        return false
    }
}
const start_scene = new THREE.Scene()

const text1 = addText("Welcome to Car Race Game", 50, 30, 35)
const text2 = addText("Up Arrow To Start", 40, 40, 35)
const text3 = addText("Use Arrow Keys for Movement and T to change View", 40, 50, 35)

let health = 100
let accelerate = false
let decelerate = false
let speed = 0
const playerCar = Car(0)
let playerX = Math.cos(angle1) * (trackRadius + 150)
let playerY = Math.sin(angle1) * (trackRadius + 150)
playerCar.position.x = playerX
playerCar.position.y = playerY
playerCar.rotation.z = angle1 - Math.PI / 2

const playerCar2 = Car(1)
playerX = Math.cos(angle2) * (trackRadius - 150)
playerY = Math.sin(angle2) * (trackRadius - 150)
playerCar2.position.x = playerX
playerCar2.position.y = playerY
playerCar2.rotation.z = angle2 - Math.PI / 2
let outtrack = false
const playerCar3 = Car(2)
playerX = Math.cos(angle3) * (trackRadius + 150)
playerY = Math.sin(angle3) * (trackRadius + 150)
playerCar3.position.x = playerX
playerCar3.position.y = playerY
playerCar3.rotation.z = angle3 - Math.PI / 2

const playerCar4 = Car(4)
playerX = Math.cos(angle4) * (trackRadius - 150)
playerY = Math.sin(angle4) * (trackRadius - 150)
playerCar4.position.x = playerX
playerCar4.position.y = playerY
playerCar4.rotation.z = angle4 - Math.PI / 2

scene.add(playerCar)
scene.add(playerCar2)
scene.add(playerCar3)
scene.add(playerCar4)

const ambient = new THREE.AmbientLight(0xffffff, 0.8)
scene.add(ambient)

const geometry = new THREE.PlaneGeometry(2 * trackWidth, 20);
const material = new THREE.MeshLambertMaterial({ color: 0xffffff });
const plane = new THREE.Mesh(geometry, material);
plane.position.x = -trackRadius
plane.position.z = 2
scene.add(plane);

const dirLight = new THREE.DirectionalLight(0xffffff, 1.0)
dirLight.position.set(100, -300, 400);
scene.add(dirLight)
const aspectRatio = window.innerWidth / window.innerHeight
const camwidth = 3500;
const camheight = camwidth / aspectRatio;
for (let i = 0; i < 10; i++) {
    gltfLoader.load('../dist/game_ready_city_building/scene.gltf', function (gltf) {
        const building = gltf.scene;
        building.scale.set(200, 200, 200)
        building.rotateX(Math.PI / 2)
        building.position.set((outerRadius + 800) * (Math.cos(Math.PI * (i / 5))), (outerRadius + 800) * (Math.sin(Math.PI * (i / 5))), 10);
        building.rotation.y = Math.PI * (i / 5)
        scene.add(building);
    });
}
for (let i = 0; i < 20; i++) {
    gltfLoader.load('../dist/barrier/scene.gltf', function (gltf) {
        const barrier = gltf.scene;
        barrier.scale.set(100, 50, 50)
        barrier.rotateX(Math.PI / 2)
        barrier.position.set((outerRadius) * (Math.cos(Math.PI * (i / 10))), (outerRadius) * (Math.sin(Math.PI * (i / 10))), 10);
        barrier.rotation.y = Math.PI * (i / 10)
        scene.add(barrier);
    });
}
for (let i = 0; i < 10; i++) {
    gltfLoader.load('../dist/barrier/scene.gltf', function (gltf) {
        const barrier = gltf.scene;
        barrier.scale.set(100, 50, 50)
        barrier.rotateX(Math.PI / 2)
        barrier.position.set((innerRadius - 40) * (Math.cos(Math.PI * (i / 5))), (innerRadius - 40) * (Math.sin(Math.PI * (i / 5))), 10);
        barrier.rotation.y = Math.PI * (i / 5)
        scene.add(barrier);
    });
}
for (let i = 0; i < 4; i++) {
    gltfLoader.load('../dist/crowd/scene.gltf', function (gltf) {
        const crowd = gltf.scene;
        crowd.rotateX(Math.PI / 2)
        crowd.position.set((outerRadius + 200) * (Math.cos(Math.PI * (i / 2))), (outerRadius + 200) * (Math.sin(Math.PI * (i / 2))), 10);
        crowd.rotation.y = -Math.PI / 2 + Math.PI * (i / 2)
        scene.add(crowd);
    });
}
gltfLoader.load('../dist/fuel_can/scene.gltf', function (gltf) {
    const can = gltf.scene;
    can.rotateX(Math.PI / 2)
    can.position.set(0, 0, 10);
    can.rotation.y = -Math.PI / 2
    window.fuelcan = can
    scene.add(window.fuelcan)
});
const camera = new THREE.PerspectiveCamera(75, window.innerWidth / window.innerHeight, 0.1, 300000);
playerX = Math.cos(camAngle) * (trackRadius)
playerY = Math.sin(camAngle) * (trackRadius)
let normalX = Math.cos(angle1) * (trackRadius)
let normalY = Math.sin(angle1) * (trackRadius)
let lasttoggle = 0
let first = true
let score = -1
let timestart = 0
let lasthit = 0;
let mileage = 200
let fuel = 100
camera.position.x = playerX
camera.position.y = playerY
camera.position.z = 400
camera.rotation.z = camAngle - Math.PI / 2
camera.up = new THREE.Vector3(0, 0, 1)
camera.lookAt(normalX, normalY, playerCar.position.z)

renderMap(camwidth, camheight * 2)
const camortho = camwidth + 2000;
const camh_ortho = camortho / aspectRatio;
const camera_orthogonal = new THREE.OrthographicCamera(
    camortho / -2,
    camortho / 2,
    camh_ortho / 2,
    camh_ortho / -2,
    0,
    3000000
)
camera_orthogonal.position.set(0, 0, 1000000)
camera_orthogonal.lookAt(0, 0, 0)
const renderer = new THREE.WebGLRenderer({ antialias: true })
renderer.setSize(window.innerWidth, window.innerHeight)
renderer.render(scene, camera)
document.getElementById("gamewindow").appendChild(renderer.domElement)
renderer.setAnimationLoop(animate)
scene.background = new THREE.Color(0x87ceeb);
start_scene.background = new THREE.Color(0x023020);
window.addEventListener("keydown", function (event) {
    if (event.key == "ArrowUp") {
        startGame()
        accelerate = true
    }
    if (event.key == "ArrowDown") {
        decelerate = true
    }
    if (event.key == "T" || event.key == 't') {
        const d1 = new Date()
        if (d1.getTime() - lasttoggle > 1000) {
            if (toggle == true) toggle = false
            else toggle = true;
            lasttoggle = d1.getTime()
        }
    }
    if (event.key == "ArrowLeft") {
        right = 0.01
    }
    if (event.key == "ArrowRight") {
        right = -0.01
    }
})

window.addEventListener("keyup", function (event) {
    if (event.key == "ArrowUp") {
        accelerate = false
    }
    if (event.key == "ArrowDown") {
        decelerate = false
    }
    if (event.key == "ArrowLeft") {
        right = 0
    }
    if (event.key == "ArrowRight") {
        right = 0
    }
})
const insetHeight = window.innerHeight / 4
const insetWidth = window.innerHeight / 4
function animate(timestamp) {
    if (flag == true && end == false) {
        if (first) {
            window.fuelcan.position.set(trackRadius, 0, 10)
            first = false
        }
        text1.innerHTML = ""
        text2.innerHTML = ""
        text3.innerHTML = ""
        if (!lastTimestamp) {
            lastTimestamp = timestamp
            return
        }
        if (flag == false) {
            timestart = timestamp
        }

        document.getElementById("score").innerHTML = Math.max(0, score)
        document.getElementById("health").innerHTML = Math.max(0, health)
        document.getElementById("fuel").innerHTML = Math.ceil(fuel)
        document.getElementById("time").innerHTML = Math.floor((timestamp - timestart) / 1000)
        const timeDelta = timestamp - lastTimestamp
        moveOtherVehicles(timeDelta);
        MovePlayer(timeDelta)
        Calculatehits()
        lastTimestamp += timeDelta
        renderer.setViewport(0, 0, window.innerWidth, window.innerHeight);
        renderer.render(scene, camera);
        renderer.setScissorTest(true);
        renderer.setScissor(16, window.innerHeight - insetHeight - 16, insetWidth, insetHeight);
        renderer.setViewport(16, window.innerHeight - insetHeight - 16, insetWidth, insetHeight);
        renderer.render(scene, camera_orthogonal)
        renderer.setScissorTest(false);
    }
    else if (end == false) {
        renderer.render(start_scene, camera_orthogonal)
    }
    else {
        renderer.render(start_scene, camera_orthogonal)
    }
};
