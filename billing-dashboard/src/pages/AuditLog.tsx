import { usePageTitle } from '../hooks/usePageTitle';
import { apiFetch } from '../utils/api';
import {  } from "../config";
import { useState, useEffect } from 'react';
import {
    Table, TableBody, TableCell, TableHead, TableHeader, TableRow
} from '../components/ui/table';
import { Card, CardHeader, CardContent } from '../components/ui/card';
import { Badge } from '../components/ui/badge';
import { Button } from '../components/ui/button';
import { Input } from '../components/ui/input';
import { Search, Download, ShieldAlert } from 'lucide-react';

interface AuditLogEntry {
    id: string;
    time: string;
    user: string;
    ip: string | null;
    action: string;
    entity: string;
    details: string;
}

export const AuditLog = () => {
    usePageTitle('Audit Log');
    const [logs, setLogs] = useState<AuditLogEntry[]>([]);
    const [searchQuery, setSearchQuery] = useState('');
    const [loading, setLoading] = useState(true);

    useEffect(() => {
        apiFetch(`/api/audit`)
            .then(res => res.json())
            .then(data => {
                setLogs(data);
                setLoading(false);
            })
            .catch(error => {
                console.error("Error fetching audit logs:", error);
                setLoading(false);
            });
    }, []);

    const filteredLogs = logs.filter(log =>
        log.user.toLowerCase().includes(searchQuery.toLowerCase()) ||
        log.action.toLowerCase().includes(searchQuery.toLowerCase()) ||
        log.entity.toLowerCase().includes(searchQuery.toLowerCase())
    );

    const getActionBadge = (action: string) => {
        switch (action) {
            case 'CREATE': return <Badge className="bg-emerald-500 hover:bg-emerald-600">CREATE</Badge>;
            case 'UPDATE': return <Badge className="bg-amber-500 hover:bg-amber-600">UPDATE</Badge>;
            case 'DELETE': return <Badge variant="destructive">DELETE</Badge>;
            case 'LOGIN': return <Badge variant="secondary">LOGIN</Badge>;
            case 'PAYMENT': return <Badge className="bg-blue-500 hover:bg-blue-600">PAYMENT</Badge>;
            default: return <Badge variant="outline">{action}</Badge>;
        }
    };

    return (
        <div className="space-y-6">
            <div className="flex flex-col sm:flex-row justify-between items-start sm:items-center space-y-4 sm:space-y-0">
                <div>
                    <h2 className="text-3xl font-bold tracking-tight">System Audit Log</h2>
                    <p className="text-muted-foreground mt-1">Immutable record of all administrative and financial actions.</p>
                </div>
                <div className="flex bg-destructive/10 text-destructive px-4 py-2 rounded-md items-center border border-destructive/20 font-medium">
                    <ShieldAlert className="mr-2 h-4 w-4" />
                    Admin & Manager Access Only
                </div>
            </div>

            <Card className="hover-lift transition-all duration-300 border-l-4 border-l-slate-400">
                <CardHeader className="pb-3 border-b">
                    <div className="flex flex-col sm:flex-row justify-between items-center space-y-2 sm:space-y-0 mt-2">
                        <div className="relative w-full sm:max-w-md">
                            <Search className="absolute left-2.5 top-2.5 h-4 w-4 text-muted-foreground" />
                            <Input
                                type="text"
                                placeholder="Search by User, Action, or Entity..."
                                className="pl-9"
                                value={searchQuery}
                                onChange={(e) => setSearchQuery(e.target.value)}
                            />
                        </div>
                        <Button variant="outline"><Download className="mr-2 h-4 w-4" /> Export CSV</Button>
                    </div>
                </CardHeader>
                <CardContent className="p-0">
                    <Table>
                        <TableHeader className="bg-muted/50 text-xs uppercase tracking-wider">
                            <TableRow>
                                <TableHead className="w-[180px]">Timestamp</TableHead>
                                <TableHead className="w-[100px]">User</TableHead>
                                <TableHead className="w-[120px]">IP Addr</TableHead>
                                <TableHead className="w-[100px]">Action</TableHead>
                                <TableHead className="w-[120px]">Entity</TableHead>
                                <TableHead>Details</TableHead>
                            </TableRow>
                        </TableHeader>
                        <TableBody className="font-mono text-sm">
                            {loading ? (
                                <TableRow>
                                    <TableCell colSpan={6} className="h-24 text-center font-sans text-muted-foreground">
                                        Loading audit logs...
                                    </TableCell>
                                </TableRow>
                            ) : filteredLogs.length === 0 ? (
                                <TableRow>
                                    <TableCell colSpan={6} className="h-24 text-center font-sans text-muted-foreground">
                                        No log sequence matching query.
                                    </TableCell>
                                </TableRow>
                            ) : (
                                filteredLogs.map((log) => (
                                    <TableRow key={log.id}>
                                        <TableCell className="text-muted-foreground">{new Date(log.time).toLocaleString()}</TableCell>
                                        <TableCell className="font-bold text-foreground">{log.user}</TableCell>
                                        <TableCell className="text-muted-foreground">{log.ip || 'N/A'}</TableCell>
                                        <TableCell>{getActionBadge(log.action)}</TableCell>
                                        <TableCell>{log.entity}</TableCell>
                                        <TableCell className="truncate max-w-[300px] hover:whitespace-normal">{log.details}</TableCell>
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
