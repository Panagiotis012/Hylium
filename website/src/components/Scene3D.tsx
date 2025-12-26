import { useRef, useState, useEffect } from 'react';
import { Canvas, useFrame } from '@react-three/fiber';
import { OrbitControls, Float } from '@react-three/drei';
import { OBJLoader } from 'three/examples/jsm/loaders/OBJLoader.js';
import * as THREE from 'three';

const HologramModel = () => {
    const meshRef = useRef<THREE.Group>(null);
    const [model, setModel] = useState<THREE.Group | null>(null);
    const [error, setError] = useState(false);

    useEffect(() => {
        const loader = new OBJLoader();
        loader.load(
            '/models/Hologram.obj',
            (obj) => {
                obj.traverse((child) => {
                    if (child instanceof THREE.Mesh) {
                        child.material = new THREE.MeshStandardMaterial({
                            color: '#00d4ff',
                            emissive: '#0891b2',
                            emissiveIntensity: 0.15,
                            transparent: true,
                            opacity: 0.95,
                            metalness: 0.5,
                            roughness: 0.4,
                            side: THREE.DoubleSide,
                        });
                    }
                });

                const box = new THREE.Box3().setFromObject(obj);
                const center = box.getCenter(new THREE.Vector3());
                const size = box.getSize(new THREE.Vector3());
                const maxDim = Math.max(size.x, size.y, size.z);
                const scale = 5 / maxDim; // VERY BIG - scale 5

                obj.position.sub(center);
                obj.scale.setScalar(scale);
                setModel(obj);
            },
            undefined,
            () => setError(true)
        );
    }, []);

    useFrame(({ clock }) => {
        if (meshRef.current) {
            meshRef.current.rotation.y = clock.getElapsedTime() * 0.25;
        }
    });

    if (error) {
        return (
            <Float speed={1.5} rotationIntensity={0.2} floatIntensity={0.5}>
                <mesh ref={meshRef as any}>
                    <icosahedronGeometry args={[3, 2]} />
                    <meshStandardMaterial color="#00d4ff" emissive="#0891b2" emissiveIntensity={0.15} wireframe />
                </mesh>
            </Float>
        );
    }

    if (!model) return null;

    return (
        <Float speed={1.5} rotationIntensity={0.15} floatIntensity={0.4}>
            <primitive ref={meshRef} object={model} />
        </Float>
    );
};

const Scene3D = () => {
    return (
        <Canvas
            style={{ width: '100%', height: '100%' }}
            camera={{ position: [0, 0, 7], fov: 50 }}
            gl={{ antialias: true, alpha: true }}
        >
            <ambientLight intensity={0.8} />
            <directionalLight position={[5, 5, 5]} intensity={1.5} />
            <pointLight position={[0, 0, 5]} intensity={2} color="#00d4ff" distance={15} />
            <HologramModel />
            <OrbitControls enableZoom={false} enablePan={false} autoRotate autoRotateSpeed={0.6} />
        </Canvas>
    );
};

export default Scene3D;
