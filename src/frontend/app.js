// --- 1. Three.js Scene Setup ---
const container = document.getElementById('canvas-wrapper');
const scene = new THREE.Scene();
const camera = new THREE.PerspectiveCamera(60, container.clientWidth / container.clientHeight, 0.1, 1000);
const renderer = new THREE.WebGLRenderer({ antialias: true });

renderer.setSize(container.clientWidth, container.clientHeight);
container.appendChild(renderer.domElement);

// Constructing 3D Cubie Mesh arrays
const cubies = [];
const masterCubeGroup = new THREE.Group();

const materials = [
    new THREE.MeshBasicMaterial({color: 0xb30000}), // Right (Red)
    new THREE.MeshBasicMaterial({color: 0xff7300}), // Left (Orange)
    new THREE.MeshBasicMaterial({color: 0xffffff}), // Up (White)
    new THREE.MeshBasicMaterial({color: 0xffd000}), // Down (Yellow)
    new THREE.MeshBasicMaterial({color: 0x00b31e}), // Front (Green)
    new THREE.MeshBasicMaterial({color: 0x001eff})  // Back (Blue)
];

for (let x = -1; x <= 1; x++) {
    for (let y = -1; y <= 1; y++) {
        for (let z = -1; z <= 1; z++) {
            const geom = new THREE.BoxGeometry(0.92, 0.92, 0.92);
            const mesh = new THREE.Mesh(geom, materials);
            mesh.position.set(x, y, z);
            masterCubeGroup.add(mesh);
            cubies.push(mesh);
        }
    }
}
scene.add(masterCubeGroup);
camera.position.set(3, 3.5, 4.5);
camera.lookAt(0,0,0);

// Animation Loop
function render() {
    requestAnimationFrame(render);
    masterCubeGroup.rotation.y += 0.003; // Soft showcase spin rotation
    renderer.render(scene, camera);
}
render();

// Window Resize handling
window.addEventListener('resize', () => {
    camera.aspect = container.clientWidth / container.clientHeight;
    camera.updateProjectionMatrix();
    renderer.setSize(container.clientWidth, container.clientHeight);
});

// --- 2. WebAssembly Module Hooks ---
let solutionSequence = [];
let currentStepIdx = 0;

Module.onRuntimeInitialized = () => {
    document.getElementById('status-display').innerText = "Wasm Solver Engine Status: Ready to Calculate.";
};

document.getElementById('solve-btn').addEventListener('click', () => {
    // Standard mock scrambled starting sequence configuration representing 54 face tiles
    const rawCubeStateStr = "UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB";
    
    document.getElementById('status-display').innerText = "Calculating optimal path...";
    
    // Bind C++ Solver using Emscripten cwrap
    const nativeSolver = Module.cwrap('solveCubeIDAStar', 'string', ['string']);
    const solutionOut = nativeSolver(rawCubeStateStr);
    
    document.getElementById('status-display').innerText = "Moves Required: " + solutionOut;
    
    if (solutionOut !== "Unsolvable" && solutionOut !== "Solved") {
        solutionSequence = solutionOut.trim().split(" ");
        currentStepIdx = 0;
        document.getElementById('next-btn').disabled = false;
    }
});

document.getElementById('next-btn').addEventListener('click', () => {
    if (currentStepIdx < solutionSequence.length) {
        const nextMove = solutionSequence[currentStepIdx];
        document.getElementById('status-display').innerText = `Executing move (${currentStepIdx + 1}/${solutionSequence.length}): ${nextMove}`;
        
        // This is where you would trigger specific layer rotations in Three.js
        
        currentStepIdx++;
        if (currentStepIdx === solutionSequence.length) {
            document.getElementById('next-btn').disabled = true;
            document.getElementById('status-display').innerText = "Cube fully restored to standard state!";
        }
    }
});

document.getElementById('scramble-btn').addEventListener('click', () => {
    // Scramble function implementation
    document.getElementById('status-display').innerText = "Cube Randomized.";
    document.getElementById('next-btn').disabled = true;
});