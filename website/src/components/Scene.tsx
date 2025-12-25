
import React, { useRef, useMemo } from 'react';
import { Canvas, useFrame } from '@react-three/fiber';
import { OrbitControls, Float, Environment } from '@react-three/drei';
import * as THREE from 'three';

const HyliumCrystal = () => {
    const meshRef = useRef<THREE.Mesh>(null);
    const innerRef = useRef<THREE.Mesh>(null);

    useFrame(({ clock }) => {
        if (meshRef.current) {
            meshRef.current.rotation.y = clock.getElapsedTime() * 0.2;
            meshRef.current.rotation.x = Math.sin(clock.getElapsedTime() * 0.15) * 0.05;
        }
        if (innerRef.current) {
            innerRef.current.rotation.y = -clock.getElapsedTime() * 0.15;
        }
    });

    return (
        <Float speed={1.2} rotationIntensity={0.2} floatIntensity={0.6}>
            <group>
                {/* Outer Crystal - Glass-like */}
                <mesh ref={meshRef} scale={2}>
                    <icosahedronGeometry args={[1, 1]} />
                    <meshPhysicalMaterial
                        color="#6366f1"
                        metalness={0.1}
                        roughness={0.1}
                        transmission={0.6}
                        thickness={1.5}
                        ior={2.4}
                        envMapIntensity={1}
                        clearcoat={1}
                        clearcoatRoughness={0.1}
                        transparent
                        opacity={0.9}
                    />
                </mesh>

                {/* Inner Core - Subtle glow */}
                <mesh ref={innerRef} scale={1.2}>
                    <icosahedronGeometry args={[1, 0]} />
                    <meshBasicMaterial
                        color="#8b5cf6"
                        transparent
                        opacity={0.3}
                    />
                </mesh>
            </group>
        </Float>
    );
};

// Ambient particle ring
const ParticleRing = () => {
    const pointsRef = useRef<THREE.Points>(null);

    const particles = useMemo(() => {
        const count = 400;
        const positions = new Float32Array(count * 3);
        for (let i = 0; i < count; i++) {
            const angle = (i / count) * Math.PI * 2;
            const radius = 3.5 + Math.random() * 0.3;
            positions[i * 3] = Math.cos(angle) * radius;
            positions[i * 3 + 1] = (Math.random() - 0.5) * 0.2;
            positions[i * 3 + 2] = Math.sin(angle) * radius;
        }
        return positions;
    }, []);

    useFrame(({ clock }) => {
        if (pointsRef.current) {
            pointsRef.current.rotation.y = clock.getElapsedTime() * 0.08;
        }
    });

    return (
        <points ref={pointsRef}>
            <bufferGeometry>
                <bufferAttribute
                    attach="attributes-position"
                    args={[particles, 3]}
                />
            </bufferGeometry>
            <pointsMaterial
                size={0.015}
                color="#a5b4fc"
                transparent
                opacity={0.5}
                sizeAttenuation
            />
        </points>
    );
};

const Scene: React.FC = () => {
    return (
        <Canvas
            style={{ height: '100%', width: '100%' }}
            camera={{ position: [0, 0, 6], fov: 50 }}
            gl={{ antialias: true, alpha: true }}
        >
            {/* Environment map for natural reflections */}
            <Environment preset="night" />

            {/* Soft ambient */}
            <ambientLight intensity={0.3} color="#e0e7ff" />

            {/* Main light - soft, from upper right */}
            <directionalLight
                position={[5, 4, 3]}
                intensity={1}
                color="#f8fafc"
            />

            <React.Suspense fallback={null}>
                <HyliumCrystal />
                <ParticleRing />
            </React.Suspense>

            <OrbitControls
                enableZoom={false}
                enablePan={false}
                autoRotate
                autoRotateSpeed={0.3}
            />
        </Canvas>
    );
};

export default Scene;
