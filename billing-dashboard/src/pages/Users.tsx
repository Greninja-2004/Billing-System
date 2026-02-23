import { usePageTitle } from '../hooks/usePageTitle';
import { apiFetch } from '../utils/api';
import { API_BASE_URL } from "../config";
import { useState, useEffect } from 'react';
import {
    Table, TableBody, TableCell, TableHead, TableHeader, TableRow
} from '../components/ui/table';
import { Card, CardHeader, CardContent } from '../components/ui/card';
import { Badge } from '../components/ui/badge';
import { Button } from '../components/ui/button';
import { Plus, MoreHorizontal, ShieldAlert } from 'lucide-react';

interface User {
    id: string;
    username: string;
    role: string;
    lastLogin: string | null;
    status: string;
}

export const Users = () => {
    usePageTitle('User Management');
    const [users, setUsers] = useState<User[]>([]);
    const [loading, setLoading] = useState(true);

    useEffect(() => {
        apiFetch(`/api/users`)
            .then(res => res.json())
            .then(data => {
                setUsers(data);
                setLoading(false);
            })
            .catch(error => {
                console.error("Error fetching users:", error);
                setLoading(false);
            });
    }, []);
    const getRoleBadge = (role: string) => {
        switch (role) {
            case 'ADMIN': return <Badge variant="destructive">ADMIN</Badge>;
            case 'MANAGER': return <Badge className="bg-purple-500 hover:bg-purple-600">MANAGER</Badge>;
            case 'BILLING': return <Badge className="bg-blue-500 hover:bg-blue-600">BILLING</Badge>;
            case 'VIEWER': return <Badge variant="secondary">VIEWER</Badge>;
            default: return <Badge variant="outline">{role}</Badge>;
        }
    };

    return (
        <div className="space-y-6">
            <div className="flex flex-col sm:flex-row justify-between items-start sm:items-center space-y-4 sm:space-y-0">
                <div>
                    <h2 className="text-3xl font-bold tracking-tight">User Management</h2>
                    <p className="text-muted-foreground mt-1">Manage system access, assign roles, and revoke privileges.</p>
                </div>
                <div className="flex space-x-2">
                    <Button><Plus className="mr-2 h-4 w-4" /> Add User</Button>
                </div>
            </div>

            <Card>
                <CardHeader className="pb-3 border-b">
                    <div className="flex items-center space-x-2">
                        <ShieldAlert className="h-5 w-5 text-muted-foreground" />
                        <span className="font-semibold">Role-Based Access Control (RBAC) configured via C++ Policy engine.</span>
                    </div>
                </CardHeader>
                <CardContent className="p-0">
                    <Table>
                        <TableHeader className="bg-muted/50">
                            <TableRow>
                                <TableHead>User ID</TableHead>
                                <TableHead>Username</TableHead>
                                <TableHead>System Role</TableHead>
                                <TableHead>Status</TableHead>
                                <TableHead>Last Login</TableHead>
                                <TableHead className="text-right w-[100px]">Actions</TableHead>
                            </TableRow>
                        </TableHeader>
                        <TableBody>
                            {loading ? (
                                <TableRow>
                                    <TableCell colSpan={6} className="h-24 text-center text-muted-foreground">
                                        Loading users...
                                    </TableCell>
                                </TableRow>
                            ) : users.length === 0 ? (
                                <TableRow>
                                    <TableCell colSpan={6} className="h-24 text-center text-muted-foreground">
                                        No users found.
                                    </TableCell>
                                </TableRow>
                            ) : (
                                users.map((user) => (
                                    <TableRow key={user.id} className={user.status === 'Deactivated' ? 'opacity-50' : ''}>
                                        <TableCell className="font-medium text-muted-foreground">{user.id}</TableCell>
                                        <TableCell className="font-bold flex items-center space-x-2">
                                            <div className="h-6 w-6 rounded-full bg-primary/20 flex items-center justify-center text-primary text-xs mr-2">
                                                {user.username.charAt(0).toUpperCase()}
                                            </div>
                                            {user.username}
                                        </TableCell>
                                        <TableCell>{getRoleBadge(user.role)}</TableCell>
                                        <TableCell>
                                            {user.status === 'Active' ? (
                                                <Badge variant="success" className="px-2 py-0">Active</Badge>
                                            ) : (
                                                <Badge variant="secondary" className="px-2 py-0">Deactivated</Badge>
                                            )}
                                        </TableCell>
                                        <TableCell className="text-muted-foreground tabular-nums">
                                            {user.lastLogin ? new Date(user.lastLogin).toLocaleString() : 'Never'}
                                        </TableCell>
                                        <TableCell className="text-right">
                                            <Button variant="ghost" size="icon">
                                                <MoreHorizontal className="h-4 w-4" />
                                            </Button>
                                        </TableCell>
                                    </TableRow>
                                ))
                            )}
                        </TableBody>
                    </Table>
                </CardContent>
            </Card>
        </div>
    );
};
