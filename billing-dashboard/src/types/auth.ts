export type Role = "ADMIN" | "MANAGER" | "VIEWER";

export interface User {
    id: string;
    username: string;
    role: Role;
}

export interface AuthState {
    user: User | null;
    token: string | null;
    isAuthenticated: boolean;
}
