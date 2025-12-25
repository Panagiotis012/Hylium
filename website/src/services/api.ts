const API_BASE = 'http://localhost:3001/api';

export interface NetworkStats {
    blockHeight: number;
    difficulty: number;
    chain: string;
    networkHashrate: number;
    connections: number;
    lastUpdated: string;
    stale?: boolean;
}

export interface ApiResponse<T> {
    data?: T;
    error?: string;
}

async function fetchApi<T>(endpoint: string, options?: RequestInit): Promise<ApiResponse<T>> {
    try {
        const response = await fetch(`${API_BASE}${endpoint}`, {
            ...options,
            headers: {
                'Content-Type': 'application/json',
                ...options?.headers,
            },
        });

        const data = await response.json();

        if (!response.ok) {
            return { error: data.error || 'Request failed' };
        }

        return { data };
    } catch (error) {
        return { error: 'Network error' };
    }
}

// Stats
export const getStats = () => fetchApi<NetworkStats>('/stats');

// Newsletter
export const subscribeNewsletter = (email: string) =>
    fetchApi<{ message: string }>('/newsletter', {
        method: 'POST',
        body: JSON.stringify({ email }),
    });

// Contact
export const submitContact = (email: string, subject: string, message: string) =>
    fetchApi<{ message: string }>('/contact', {
        method: 'POST',
        body: JSON.stringify({ email, subject, message }),
    });

// Downloads
export interface Download {
    id: string;
    name: string;
    version: string;
    platform: string;
    url: string;
    checksum?: string;
}

export const getDownloads = () => fetchApi<Download[]>('/downloads');
