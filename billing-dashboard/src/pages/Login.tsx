import { API_BASE_URL } from "../config";
import { useState } from 'react';
import { useAuthStore } from '../store/auth';
import { useNavigate } from 'react-router-dom';
import { Button } from '../components/ui/button';
import { Input } from '../components/ui/input';
import { Label } from '../components/ui/label';
import { Card, CardHeader, CardTitle, CardDescription, CardContent, CardFooter } from '../components/ui/card';
// Removed type User since it is no longer used for MOCK_USERS

// Removed static MOCK_USERS since we now authenticate via real API

export const Login = () => {
    const [username, setUsername] = useState('admin');
    const [password, setPassword] = useState('admin123');
    const [error, setError] = useState(false);
    const login = useAuthStore((state) => state.login);
    const navigate = useNavigate();

    const handleLogin = async (e: React.FormEvent) => {
        e.preventDefault();

        try {
            const res = await fetch(`${API_BASE_URL}/api/login`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ username, password })
            });
            const data = await res.json();

            if (res.ok && data.success) {
                login(data.user, data.token);
                setError(false);
                navigate('/');
            } else {
                throw new Error(data.message || 'Login failed');
            }
        } catch (err) {
            console.error(err);
            setError(true);
            setTimeout(() => setError(false), 500); // Remove shake class after animation
        }
    };

    return (
        <div className="min-h-screen flex items-center justify-center bg-muted/40 p-4">
            <Card className={`w-full max-w-md ${error ? 'animate-shake border-destructive/50' : ''}`}>
                <CardHeader className="space-y-1 text-center">
                    <div className="flex justify-center mb-4">
                        <div className="h-12 w-12 rounded-xl bg-primary flex items-center justify-center text-primary-foreground font-bold text-xl shadow-inner">
                            BS
                        </div>
                    </div>
                    <CardTitle className="text-2xl tracking-tight">Billing System Pro</CardTitle>
                    <CardDescription>Enter your credentials to access the dashboard</CardDescription>
                </CardHeader>
                <form onSubmit={handleLogin}>
                    <CardContent className="space-y-4">
                        <div className="space-y-2">
                            <Label htmlFor="username">Username</Label>
                            <Input
                                id="username"
                                autoCapitalize="none"
                                autoComplete="username"
                                value={username}
                                onChange={(e) => setUsername(e.target.value)}
                                required
                            />
                        </div>
                        <div className="space-y-2">
                            <div className="flex items-center justify-between">
                                <Label htmlFor="password">Password</Label>
                            </div>
                            <Input
                                id="password"
                                type="password"
                                autoComplete="current-password"
                                value={password}
                                onChange={(e) => setPassword(e.target.value)}
                                required
                            />
                        </div>
                        {error && (
                            <p className="text-sm text-destructive font-medium text-center">
                                Invalid username or password
                            </p>
                        )}
                        <div className="text-xs text-muted-foreground bg-muted p-3 rounded-md">
                            <p className="font-semibold mb-1">Demo Accounts:</p>
                            <ul className="list-disc pl-4 space-y-1">
                                <li>admin / admin123 (Full Access)</li>
                                <li>manager / manager123</li>
                                <li>viewer / readonly</li>
                            </ul>
                        </div>
                    </CardContent>
                    <CardFooter>
                        <Button type="submit" className="w-full">
                            Sign In
                        </Button>
                    </CardFooter>
                </form>
            </Card>
        </div>
    );
};
