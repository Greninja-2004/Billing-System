/**
 * Authenticated fetch utility — automatically attaches the JWT token
 * stored in Zustand auth store to every request.
 */

import { API_BASE_URL } from '../config';

function getToken(): string | null {
    try {
        const stored = localStorage.getItem('billing-auth-storage');
        if (!stored) return null;
        const parsed = JSON.parse(stored);
        return parsed?.state?.token || null;
    } catch {
        return null;
    }
}

export async function apiFetch(path: string, options: RequestInit = {}): Promise<Response> {
    const token = getToken();
    const headers: Record<string, string> = {
        'Content-Type': 'application/json',
        ...(options.headers as Record<string, string> || {}),
    };
    if (token) {
        headers['Authorization'] = `Bearer ${token}`;
    }
    return fetch(`${API_BASE_URL}${path}`, { ...options, headers });
}
