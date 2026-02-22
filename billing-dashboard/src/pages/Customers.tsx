import { API_BASE_URL } from "../config";
import { useState, useMemo, useEffect } from 'react';
import {
    Table, TableBody, TableCell, TableHead, TableHeader, TableRow
} from '../components/ui/table';
import { Button } from '../components/ui/button';
import { Input } from '../components/ui/input';
import { Badge } from '../components/ui/badge';
import { Card, CardHeader, CardContent } from '../components/ui/card';
import { Search, Plus, MoreHorizontal, Filter } from 'lucide-react';

interface Customer {
    id: string;
    name: string;
    email: string;
    tier: string;
    creditScore: number;
    lifetimeValue: number;
    status: string;
    joined: string;
}

export const Customers = () => {
    const [customers, setCustomers] = useState<Customer[]>([]);
    const [searchQuery, setSearchQuery] = useState('');
    const [loading, setLoading] = useState(true);

    useEffect(() => {
        fetch(`${API_BASE_URL}/api/customers`)
            .then(res => res.json())
            .then(data => {
                setCustomers(data);
                setLoading(false);
            })
            .catch(error => {
                console.error("Error fetching customers:", error);
                setLoading(false);
            });
    }, []);

    // Client-side filtering
    const filteredCustomers = useMemo(() => {
        return customers.filter(c =>
            c.name.toLowerCase().includes(searchQuery.toLowerCase()) ||
            c.id.toLowerCase().includes(searchQuery.toLowerCase()) ||
            c.email.toLowerCase().includes(searchQuery.toLowerCase())
        );
    }, [customers, searchQuery]);

    const getTierBadge = (tier: string) => {
        switch (tier) {
            case 'Enterprise': return <Badge className="bg-purple-500 hover:bg-purple-600">Enterprise</Badge>;
            case 'Gold': return <Badge className="bg-amber-500 hover:bg-amber-600">Gold</Badge>;
            case 'Silver': return <Badge className="bg-slate-400 hover:bg-slate-500">Silver</Badge>;
            default: return <Badge className="bg-orange-700 hover:bg-orange-800">Bronze</Badge>;
        }
    };

    const getStatusBadge = (status: string) => {
        return status === 'Active'
            ? <Badge variant="success">Active</Badge>
            : <Badge variant="destructive">Suspended</Badge>;
    };

    return (
        <div className="space-y-6">
            <div className="flex flex-col sm:flex-row justify-between items-start sm:items-center space-y-4 sm:space-y-0">
                <div>
                    <h2 className="text-3xl font-bold tracking-tight">Customers</h2>
                    <p className="text-muted-foreground mt-1">Manage your customer accounts, tiers, and credit profiles.</p>
                </div>
                <Button>
                    <Plus className="mr-2 h-4 w-4" /> Add Customer
                </Button>
            </div>

            <Card>
                <CardHeader className="pb-3 border-b">
                    <div className="flex flex-col sm:flex-row justify-between items-center space-y-2 sm:space-y-0 mt-2">
                        <div className="relative w-full sm:max-w-sm">
                            <Search className="absolute left-2.5 top-2.5 h-4 w-4 text-muted-foreground" />
                            <Input
                                type="text"
                                placeholder="Search customers..."
                                className="pl-9"
                                value={searchQuery}
                                onChange={(e) => setSearchQuery(e.target.value)}
                            />
                        </div>
                        <Button variant="outline" className="w-full sm:w-auto">
                            <Filter className="mr-2 h-4 w-4" /> Filters
                        </Button>
                    </div>
                </CardHeader>
                <CardContent className="p-0">
                    <Table>
                        <TableHeader>
                            <TableRow className="bg-muted/50">
                                <TableHead className="w-[100px]">ID</TableHead>
                                <TableHead>Customer</TableHead>
                                <TableHead>Tier</TableHead>
                                <TableHead>Status</TableHead>
                                <TableHead className="text-right">Credit Score</TableHead>
                                <TableHead className="text-right">Lifetime Value</TableHead>
                                <TableHead className="w-[50px]"></TableHead>
                            </TableRow>
                        </TableHeader>
                        <TableBody>
                            {loading ? (
                                <TableRow>
                                    <TableCell colSpan={7} className="h-24 text-center text-muted-foreground">
                                        Loading customers...
                                    </TableCell>
                                </TableRow>
                            ) : filteredCustomers.length === 0 ? (
                                <TableRow>
                                    <TableCell colSpan={7} className="h-24 text-center text-muted-foreground">
                                        No customers found matching "{searchQuery}".
                                    </TableCell>
                                </TableRow>
                            ) : (
                                filteredCustomers.map((customer) => (
                                    <TableRow key={customer.id}>
                                        <TableCell className="font-medium text-muted-foreground">{customer.id}</TableCell>
                                        <TableCell>
                                            <div className="flex flex-col">
                                                <span className="font-semibold">{customer.name}</span>
                                                <span className="text-xs text-muted-foreground">{customer.email}</span>
                                            </div>
                                        </TableCell>
                                        <TableCell>{getTierBadge(customer.tier)}</TableCell>
                                        <TableCell>{getStatusBadge(customer.status)}</TableCell>
                                        <TableCell className="text-right font-mono">
                                            <span className={customer.creditScore > 700 ? 'text-emerald-500' : customer.creditScore < 600 ? 'text-destructive' : ''}>
                                                {customer.creditScore}
                                            </span>
                                        </TableCell>
                                        <TableCell className="text-right font-medium">
                                            ${customer.lifetimeValue.toLocaleString(undefined, { minimumFractionDigits: 2, maximumFractionDigits: 2 })}
                                        </TableCell>
                                        <TableCell>
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
