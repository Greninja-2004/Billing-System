import { API_BASE_URL } from "../config";
import { useState } from 'react';
import { useAuthStore } from '../store/auth';
import { useNavigate } from 'react-router-dom';
import { Github, Apple } from 'lucide-react';
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
                navigate('/dashboard');
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
                    <CardFooter className="flex-col gap-4">
                        <Button type="submit" className="w-full">
                            Sign In
                        </Button>
                        <div className="relative w-full">
                            <div className="absolute inset-0 flex items-center">
                                <span className="w-full border-t" />
                            </div>
                            <div className="relative flex justify-center text-xs uppercase">
                                <span className="bg-card px-2 text-muted-foreground">Or continue with</span>
                            </div>
                        </div>
                        <div className="grid grid-cols-3 gap-4 w-full">
                            <Button variant="outline" className="w-full" onClick={(e) => { e.preventDefault(); alert("GitHub Login Preview") }}>
                                <Github className="h-5 w-5" />
                            </Button>
                            <Button variant="outline" className="w-full" onClick={(e) => { e.preventDefault(); alert("Apple Login Preview") }}>
                                <Apple className="h-5 w-5 fill-current" />
                            </Button>
                            <Button variant="outline" className="w-full" onClick={(e) => { e.preventDefault(); alert("Google Login Preview") }}>
                                <svg className="h-5 w-5" viewBox="0 0 24 24">
                                    <path d="M22.56 12.25c0-.78-.07-1.53-.2-2.25H12v4.26h5.92c-.26 1.37-1.04 2.53-2.21 3.31v2.77h3.57c2.08-1.92 3.28-4.74 3.28-8.09z" fill="#4285F4" />
                                    <path d="M12 23c2.97 0 5.46-.98 7.28-2.66l-3.57-2.77c-.98.66-2.23 1.06-3.71 1.06-2.86 0-5.29-1.93-6.16-4.53H2.18v2.84C3.99 20.53 7.7 23 12 23z" fill="#34A853" />
                                    <path d="M5.84 14.09c-.22-.66-.35-1.36-.35-2.09s.13-1.43.35-2.09V7.07H2.18C1.43 8.55 1 10.22 1 12s.43 3.45 1.18 4.93l2.85-2.22.81-.62z" fill="#FBBC05" />
                                    <path d="M12 5.38c1.62 0 3.06.56 4.21 1.64l3.15-3.15C17.45 2.09 14.97 1 12 1 7.7 1 3.99 3.47 2.18 7.07l3.66 2.84c.87-2.6 3.3-4.53 6.16-4.53z" fill="#EA4335" />
                                </svg>
                            </Button>
                        </div>
                    </CardFooter>
                </form>
            </Card>
        </div>
    );
};
