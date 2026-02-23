import { apiFetch } from '../utils/api';
import { API_BASE_URL } from "../config";
import { useState, useMemo, useEffect } from 'react';
import {
    Table, TableBody, TableCell, TableHead, TableHeader, TableRow
} from '../components/ui/table';
import { Button } from '../components/ui/button';
import { Input } from '../components/ui/input';
import { Badge } from '../components/ui/badge';
import { Card, CardHeader, CardContent } from '../components/ui/card';
import { Search, Plus, MoreHorizontal, Download, FileCheck, Mail, Loader2, CreditCard, AlertCircle } from 'lucide-react';
import { format } from 'date-fns';
import {
    DropdownMenu,
    DropdownMenuContent,
    DropdownMenuItem,
    DropdownMenuLabel,
    DropdownMenuSeparator,
    DropdownMenuTrigger,
} from '../components/ui/dropdown-menu';

// TypeScript type for the Razorpay constructor (loaded from CDN)
declare const Razorpay: any;

interface Invoice {
    id: string;
    customerName: string;
    amount: number;
    due_date: string;
    status: string;
    type: string;
}

export const Invoices = () => {
    const [invoices, setInvoices] = useState<Invoice[]>([]);
    const [searchQuery, setSearchQuery] = useState('');
    const [statusFilter, setStatusFilter] = useState<string | null>(null);
    const [loading, setLoading] = useState(true);
    const [processingPaymentId, setProcessingPaymentId] = useState<string | null>(null);

    useEffect(() => {
        apiFetch(`/api/invoices`)
            .then(res => res.json())
            .then(data => {
                setInvoices(data);
                setLoading(false);
            })
            .catch(error => {
                console.error("Error fetching invoices:", error);
                setLoading(false);
            });
    }, []);

    // Client-side filtering
    const filteredInvoices = useMemo(() => {
        return invoices.filter(inv => {
            const matchesSearch = inv.customerName.toLowerCase().includes(searchQuery.toLowerCase()) ||
                inv.id.toLowerCase().includes(searchQuery.toLowerCase());
            const matchesStatus = statusFilter ? inv.status === statusFilter : true;
            return matchesSearch && matchesStatus;
        });
    }, [invoices, searchQuery, statusFilter]);

    const getStatusBadge = (status: string) => {
        switch (status) {
            case 'Paid': return <Badge variant="success">Paid</Badge>;
            case 'Pending': return <Badge variant="warning">Pending</Badge>;
            case 'Overdue': return <Badge variant="destructive">Overdue</Badge>;
            default: return <Badge variant="secondary">Draft</Badge>;
        }
    };

    const handlePayment = async (invoice: Invoice) => {
        try {
            setProcessingPaymentId(invoice.id);

            // Step 1: Create Razorpay order on backend
            const res = await apiFetch(`/api/create-razorpay-order`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({
                    invoiceId: invoice.id,
                    amount: invoice.amount,
                    customerName: invoice.customerName,
                }),
            });
            const order = await res.json();

            if (order.error) {
                alert('Failed to create payment order. Please try again.');
                setProcessingPaymentId(null);
                return;
            }

            // Step 2: Open Razorpay payment modal
            const options = {
                key: order.keyId,
                amount: order.amount,
                currency: order.currency,
                name: 'Billing Pro',
                description: `Payment for Invoice ${invoice.id}`,
                order_id: order.orderId,
                handler: async (response: any) => {
                    // Step 3: Verify payment on backend
                    const verify = await apiFetch(`/api/verify-razorpay-payment`, {
                        method: 'POST',
                        headers: { 'Content-Type': 'application/json' },
                        body: JSON.stringify({
                            razorpay_order_id: response.razorpay_order_id,
                            razorpay_payment_id: response.razorpay_payment_id,
                            razorpay_signature: response.razorpay_signature,
                        }),
                    });
                    const result = await verify.json();
                    if (result.success) {
                        // Update invoice status in local state
                        setInvoices(prev => prev.map(inv =>
                            inv.id === invoice.id ? { ...inv, status: 'Paid' } : inv
                        ));
                        alert(`✅ Payment successful! Payment ID: ${response.razorpay_payment_id}`);
                    } else {
                        alert('⚠️ Payment could not be verified. Please contact support.');
                    }
                    setProcessingPaymentId(null);
                },
                prefill: { name: invoice.customerName },
                theme: { color: '#3b82f6' },
                modal: {
                    ondismiss: () => setProcessingPaymentId(null),
                },
            };

            const rzp = new Razorpay(options);
            rzp.open();

        } catch (error) {
            console.error('Payment Error:', error);
            alert('An unexpected error occurred during payment.');
            setProcessingPaymentId(null);
        }
    };

    const overdueCount = invoices.filter(i => i.status === 'Overdue').length;
    const overdueTotal = invoices.filter(i => i.status === 'Overdue').reduce((s, i) => s + i.amount, 0);

    return (
        <div className="space-y-6">
            <div className="flex flex-col sm:flex-row justify-between items-start sm:items-center space-y-4 sm:space-y-0">
                <div>
                    <h2 className="text-3xl font-bold tracking-tight">Invoices</h2>
                    <p className="text-muted-foreground mt-1">Manage billing, track overdues, and send reminders.</p>
                </div>
                <div className="flex space-x-2">
                    <Button variant="outline" className="hover-lift" onClick={() => window.print()}><Download className="mr-2 h-4 w-4" /> Export</Button>
                    <Button className="hover-lift" onClick={() => alert('Opening New Invoice Modal...')}><Plus className="mr-2 h-4 w-4" /> New Invoice</Button>
                </div>
            </div>

            {/* Overdue Alert Banner */}
            {overdueCount > 0 && (
                <div className="flex flex-col sm:flex-row items-start sm:items-center justify-between gap-3 p-4 rounded-xl border border-red-500/30 bg-red-500/5 animate-slide-up">
                    <div className="flex items-center gap-3">
                        <AlertCircle className="h-5 w-5 text-red-500 flex-shrink-0" />
                        <div>
                            <p className="font-semibold text-red-600">{overdueCount} Overdue Invoice{overdueCount > 1 ? 's' : ''}</p>
                            <p className="text-sm text-muted-foreground">Total outstanding: <span className="font-bold text-foreground">${overdueTotal.toLocaleString(undefined, { minimumFractionDigits: 2 })}</span></p>
                        </div>
                    </div>
                    <Button size="sm" className="bg-red-500 hover:bg-red-600 text-white gap-2 flex-shrink-0" onClick={() => setStatusFilter('Overdue')}>
                        <CreditCard className="h-4 w-4" /> Pay All Overdue
                    </Button>
                </div>
            )}

            <Card className="hover-lift transition-all duration-300">
                <CardHeader className="pb-3 border-b">
                    <div className="flex flex-col space-y-4">
                        <div className="flex flex-col sm:flex-row justify-between items-center space-y-2 sm:space-y-0 mt-2">
                            <div className="relative w-full sm:max-w-md">
                                <Search className="absolute left-2.5 top-2.5 h-4 w-4 text-muted-foreground" />
                                <Input
                                    type="text"
                                    placeholder="Search invoice ID or customer name..."
                                    className="pl-9"
                                    value={searchQuery}
                                    onChange={(e) => setSearchQuery(e.target.value)}
                                />
                            </div>
                        </div>

                        {/* Status Chips Filter */}
                        <div className="flex flex-wrap gap-2">
                            <Badge
                                variant={statusFilter === null ? 'default' : 'outline'}
                                className="cursor-pointer"
                                onClick={() => setStatusFilter(null)}
                            >
                                All
                            </Badge>
                            <Badge
                                variant={statusFilter === 'Paid' ? 'success' : 'outline'}
                                className={statusFilter !== 'Paid' ? 'text-emerald-600 border-emerald-200 cursor-pointer' : 'cursor-pointer'}
                                onClick={() => setStatusFilter('Paid')}
                            >
                                Paid
                            </Badge>
                            <Badge
                                variant={statusFilter === 'Pending' ? 'warning' : 'outline'}
                                className={statusFilter !== 'Pending' ? 'text-amber-600 border-amber-200 cursor-pointer' : 'cursor-pointer'}
                                onClick={() => setStatusFilter('Pending')}
                            >
                                Pending
                            </Badge>
                            <Badge
                                variant={statusFilter === 'Overdue' ? 'destructive' : 'outline'}
                                className={statusFilter !== 'Overdue' ? 'text-red-600 border-red-200 cursor-pointer' : 'cursor-pointer'}
                                onClick={() => setStatusFilter('Overdue')}
                            >
                                Overdue
                            </Badge>
                        </div>
                    </div>
                </CardHeader>
                <CardContent className="p-0">
                    <Table>
                        <TableHeader>
                            <TableRow className="bg-muted/50">
                                <TableHead className="w-[180px]">Invoice ID</TableHead>
                                <TableHead>Customer</TableHead>
                                <TableHead>Due Date</TableHead>
                                <TableHead>Type</TableHead>
                                <TableHead>Status</TableHead>
                                <TableHead className="text-right">Amount</TableHead>
                                <TableHead className="text-right w-[150px]">Actions</TableHead>
                            </TableRow>
                        </TableHeader>
                        <TableBody>
                            {loading ? (
                                <TableRow>
                                    <TableCell colSpan={7} className="h-32 text-center text-muted-foreground">
                                        Loading invoices...
                                    </TableCell>
                                </TableRow>
                            ) : filteredInvoices.length === 0 ? (
                                <TableRow>
                                    <TableCell colSpan={7} className="h-32 text-center text-muted-foreground">
                                        <div className="flex flex-col items-center justify-center space-y-2">
                                            <FileCheck className="h-8 w-8 text-muted-foreground/50" />
                                            <p>No invoices found matching criteria.</p>
                                        </div>
                                    </TableCell>
                                </TableRow>
                            ) : (
                                filteredInvoices.map((inv) => (
                                    <TableRow key={inv.id} className="hover:bg-muted/50 transition-colors">
                                        <TableCell className="font-medium text-foreground">{inv.id}</TableCell>
                                        <TableCell className="font-semibold">{inv.customerName}</TableCell>
                                        <TableCell className="text-muted-foreground">
                                            {format(new Date(inv.due_date), 'MMM dd, yyyy')}
                                        </TableCell>
                                        <TableCell className="text-muted-foreground">{inv.type}</TableCell>
                                        <TableCell>{getStatusBadge(inv.status)}</TableCell>
                                        <TableCell className="text-right font-bold">
                                            ${inv.amount.toLocaleString(undefined, { minimumFractionDigits: 2, maximumFractionDigits: 2 })}
                                        </TableCell>
                                        <TableCell className="text-right">
                                            <div className="flex justify-end items-center gap-2">
                                                {/* Prominent Pay Now button for Pending/Overdue */}
                                                {(inv.status === 'Pending' || inv.status === 'Overdue') && (
                                                    <Button
                                                        size="sm"
                                                        variant={inv.status === 'Overdue' ? 'destructive' : 'default'}
                                                        className="gap-1.5 hover-lift"
                                                        disabled={processingPaymentId === inv.id}
                                                        onClick={() => handlePayment(inv)}
                                                    >
                                                        {processingPaymentId === inv.id ? (
                                                            <><Loader2 className="h-3.5 w-3.5 animate-spin" /> Processing...</>
                                                        ) : (
                                                            <><CreditCard className="h-3.5 w-3.5" /> Pay Now</>
                                                        )}
                                                    </Button>
                                                )}

                                                {/* Reminder for overdue */}
                                                {inv.status === 'Overdue' && (
                                                    <Button variant="ghost" size="icon" title="Send Reminder" className="text-amber-500 hover:bg-amber-500/10 h-8 w-8">
                                                        <Mail className="h-4 w-4" />
                                                    </Button>
                                                )}

                                                <DropdownMenu>
                                                    <DropdownMenuTrigger asChild>
                                                        <Button variant="ghost" size="icon" className="h-8 w-8">
                                                            <MoreHorizontal className="h-4 w-4" />
                                                        </Button>
                                                    </DropdownMenuTrigger>
                                                    <DropdownMenuContent align="end">
                                                        <DropdownMenuLabel>Actions</DropdownMenuLabel>
                                                        <DropdownMenuItem onClick={() => alert(`Viewing ${inv.id}`)}>View Details</DropdownMenuItem>
                                                        <DropdownMenuSeparator />
                                                        <DropdownMenuItem onClick={() => window.print()}>Download PDF</DropdownMenuItem>
                                                        <DropdownMenuItem onClick={() => alert(`Sending reminder for ${inv.id}`)}>Send Reminder</DropdownMenuItem>
                                                    </DropdownMenuContent>
                                                </DropdownMenu>
                                            </div>
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
