import { usePageTitle } from '../hooks/usePageTitle';
import { apiFetch } from '../utils/api';
import {  } from "../config";
import { useState, useEffect } from 'react';
import {
    Table, TableBody, TableCell, TableHead, TableHeader, TableRow
} from '../components/ui/table';
import { Button } from '../components/ui/button';
import { Badge } from '../components/ui/badge';
import { Card, CardHeader, CardTitle, CardContent } from '../components/ui/card';
import { RefreshCcw, CreditCard, Download, Search, CreditCard as CardIcon, TrendingDown, RotateCcw } from 'lucide-react';
import { Input } from '../components/ui/input';


interface Payment {
    id: string;
    invoiceId: string;
    customerName: string;
    method: string;
    amount: number;
    status: string;
    date: string;
    fraudFlag: boolean;
}

export const Payments = () => {
    usePageTitle('Payments');
    const [payments, setPayments] = useState<Payment[]>([]);
    const [searchQuery, setSearchQuery] = useState('');
    const [loading, setLoading] = useState(true);

    useEffect(() => {
        apiFetch(`/api/payments`)
            .then(res => res.json())
            .then(data => {
                setPayments(Array.isArray(data) ? data : []);
                setLoading(false);
            })
            .catch(error => {
                console.error("Error fetching payments:", error);
                setLoading(false);
            });
    }, []);

    const filteredPayments = payments.filter(p =>
        p.id.toLowerCase().includes(searchQuery.toLowerCase()) ||
        p.customerName?.toLowerCase().includes(searchQuery.toLowerCase()) ||
        p.invoiceId.toLowerCase().includes(searchQuery.toLowerCase())
    );

    // Compute live stats from actual data
    const today = new Date().toDateString();
    const todayVolume = payments
        .filter(p => new Date(p.date).toDateString() === today && p.status === 'Completed')
        .reduce((sum, p) => sum + p.amount, 0);
    const failedCount = payments.filter(p => p.status === 'Failed').length;
    const refundsTotal = payments
        .filter(p => p.status === 'Refunded')
        .reduce((sum, p) => sum + p.amount, 0);

    const getStatusBadge = (status: string) => {
        switch (status) {
            case 'Completed': return <Badge variant="success">Completed</Badge>;
            case 'Refunded': return <Badge variant="secondary">Refunded</Badge>;
            case 'Failed': return <Badge variant="destructive">Failed</Badge>;
            case 'Partial': return <Badge className="bg-indigo-500 hover:bg-indigo-600">Partial</Badge>;
            default: return <Badge variant="outline">{status}</Badge>;
        }
    };

    const getMethodIcon = (method: string) => {
        switch (method) {
            case 'Credit Card': return <CreditCard className="h-4 w-4 mr-2 text-blue-500" />;
            case 'Bank Transfer': return <RefreshCcw className="h-4 w-4 mr-2 text-emerald-500" />;
            default: return <CreditCard className="h-4 w-4 mr-2 text-purple-500" />;
        }
    };

    return (
        <div className="space-y-6">
            <div className="flex flex-col sm:flex-row justify-between items-start sm:items-center space-y-4 sm:space-y-0">
                <div>
                    <h2 className="text-3xl font-bold tracking-tight">Payments</h2>
                    <p className="text-muted-foreground mt-1">Review your transaction history and refunds.</p>
                </div>
                <Button variant="outline" className="hover-lift" onClick={() => window.print()}>
                    <Download className="mr-2 h-4 w-4" /> Export Ledger
                </Button>
            </div>

            {/* Live stats cards */}
            <div className="grid gap-4 md:grid-cols-3 mb-6">
                <Card className="hover-lift transition-all duration-300">
                    <CardHeader className="flex flex-row items-center justify-between pb-2">
                        <CardTitle className="text-sm font-medium">Today's Volume</CardTitle>
                        <CardIcon className="h-4 w-4 text-muted-foreground" />
                    </CardHeader>
                    <CardContent>
                        <div className="text-2xl font-bold">
                            {payments.length === 0 ? '—' : todayVolume > 0
                                ? `$${todayVolume.toLocaleString(undefined, { minimumFractionDigits: 2 })}`
                                : '$0.00'}
                        </div>
                        <p className="text-xs text-muted-foreground">
                            {payments.filter(p => new Date(p.date).toDateString() === today).length} transactions today
                        </p>
                    </CardContent>
                </Card>
                <Card className="hover-lift transition-all duration-300">
                    <CardHeader className="flex flex-row items-center justify-between pb-2">
                        <CardTitle className="text-sm font-medium">Failed Transactions</CardTitle>
                        <TrendingDown className="h-4 w-4 text-muted-foreground" />
                    </CardHeader>
                    <CardContent>
                        <div className={`text-2xl font-bold ${failedCount > 0 ? 'text-destructive' : ''}`}>
                            {failedCount}
                        </div>
                        <p className="text-xs text-muted-foreground">
                            {failedCount === 0 ? (payments.length === 0 ? 'No transactions yet' : 'All clear ✅') : 'Needs attention'}
                        </p>
                    </CardContent>
                </Card>
                <Card className="hover-lift transition-all duration-300">
                    <CardHeader className="flex flex-row items-center justify-between pb-2">
                        <CardTitle className="text-sm font-medium">Refunds Issued</CardTitle>
                        <RotateCcw className="h-4 w-4 text-muted-foreground" />
                    </CardHeader>
                    <CardContent>
                        <div className="text-2xl font-bold text-muted-foreground">
                            {refundsTotal > 0
                                ? `$${refundsTotal.toLocaleString(undefined, { minimumFractionDigits: 2 })}`
                                : '—'}
                        </div>
                        <p className="text-xs text-muted-foreground">
                            {payments.filter(p => p.status === 'Refunded').length} refund(s)
                        </p>
                    </CardContent>
                </Card>
            </div>

            <Card className="hover-lift transition-all duration-300">
                <CardHeader className="pb-3 border-b">
                    <div className="relative w-full sm:max-w-md mt-2">
                        <Search className="absolute left-2.5 top-2.5 h-4 w-4 text-muted-foreground" />
                        <Input
                            type="text"
                            placeholder="Search by Payment ID, Invoice, or Customer..."
                            className="pl-9"
                            value={searchQuery}
                            onChange={(e) => setSearchQuery(e.target.value)}
                        />
                    </div>
                </CardHeader>
                <CardContent className="p-0">
                    <Table>
                        <TableHeader className="bg-muted/50">
                            <TableRow>
                                <TableHead>Payment ID</TableHead>
                                <TableHead>Date / Time</TableHead>
                                <TableHead>Customer</TableHead>
                                <TableHead>Method</TableHead>
                                <TableHead>Status</TableHead>
                                <TableHead className="text-right">Amount</TableHead>
                                <TableHead className="text-right w-[120px]">Action</TableHead>
                            </TableRow>
                        </TableHeader>
                        <TableBody>
                            {loading ? (
                                <TableRow>
                                    <TableCell colSpan={7} className="h-24 text-center text-muted-foreground">
                                        Loading payments...
                                    </TableCell>
                                </TableRow>
                            ) : filteredPayments.length === 0 ? (
                                <TableRow>
                                    <TableCell colSpan={7} className="h-32 text-center">
                                        <div className="flex flex-col items-center gap-2 text-muted-foreground">
                                            <CardIcon className="h-8 w-8 opacity-30" />
                                            <p className="font-medium">No payments yet</p>
                                            <p className="text-xs">Payments will appear here after you pay an invoice.</p>
                                        </div>
                                    </TableCell>
                                </TableRow>
                            ) : (
                                filteredPayments.map((payment) => (
                                    <TableRow key={payment.id} className="hover:bg-muted/50 transition-colors">
                                        <TableCell className="font-medium text-foreground">{payment.id}</TableCell>
                                        <TableCell className="text-muted-foreground whitespace-nowrap">{new Date(payment.date).toLocaleString()}</TableCell>
                                        <TableCell>
                                            <div className="flex flex-col">
                                                <span className="font-semibold">{payment.customerName}</span>
                                                <span className="text-xs text-muted-foreground">{payment.invoiceId}</span>
                                            </div>
                                        </TableCell>
                                        <TableCell>
                                            <div className="flex items-center">
                                                {getMethodIcon(payment.method)}
                                                <span className="text-muted-foreground">{payment.method}</span>
                                            </div>
                                        </TableCell>
                                        <TableCell>
                                            <div className="flex items-center space-x-2">
                                                {getStatusBadge(payment.status)}
                                                {payment.fraudFlag && <Badge variant="destructive" className="text-[10px] px-1 py-0">FRAUD ALERT</Badge>}
                                            </div>
                                        </TableCell>
                                        <TableCell className="text-right font-bold">
                                            ${payment.amount.toLocaleString(undefined, { minimumFractionDigits: 2, maximumFractionDigits: 2 })}
                                        </TableCell>
                                        <TableCell className="text-right">
                                            {payment.status === 'Completed' ? (
                                                <Button variant="outline" size="sm" className="h-7 text-xs">Refund</Button>
                                            ) : payment.status === 'Failed' ? (
                                                <Button variant="default" size="sm" className="h-7 text-xs">Retry</Button>
                                            ) : null}
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
