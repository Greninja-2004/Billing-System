import { API_BASE_URL } from "../config";
import { useState, useMemo, useEffect } from 'react';
import {
    Table, TableBody, TableCell, TableHead, TableHeader, TableRow
} from '../components/ui/table';
import { Button } from '../components/ui/button';
import { Input } from '../components/ui/input';
import { Badge } from '../components/ui/badge';
import { Card, CardHeader, CardContent } from '../components/ui/card';
import { Search, Plus, MoreHorizontal, Download, FileCheck, Mail, Loader2, CreditCard } from 'lucide-react';
import { format } from 'date-fns';
import { loadStripe } from '@stripe/stripe-js';
import {
    DropdownMenu,
    DropdownMenuContent,
    DropdownMenuItem,
    DropdownMenuLabel,
    DropdownMenuSeparator,
    DropdownMenuTrigger,
} from '../components/ui/dropdown-menu';

// Replace with a real publishable key for production
const stripePromise = loadStripe('pk_test_51OzA...fakeTestKey...replaceWithRealLater');

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
        fetch(`${API_BASE_URL}/api/invoices`)
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
            const stripe = await stripePromise;

            const response = await fetch(`${API_BASE_URL}/api/create-checkout-session`, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify({
                    invoiceId: invoice.id,
                    amount: invoice.amount,
                    customerName: invoice.customerName,
                }),
            });

            const session = await response.json();

            if (session.error) {
                console.error("Stripe Error:", session.error);
                alert("Failed to initialize payment session.");
                setProcessingPaymentId(null);
                return;
            }

            // Redirect to Stripe Checkout
            // @ts-ignore - The official stripe-js library has a typing issue with redirectToCheckout in certain module resolution strategies, but it works perfectly at runtime.
            const result = await stripe?.redirectToCheckout({
                sessionId: session.id,
            });

            if (result?.error) {
                console.error("Stripe Redirect Error:", result.error);
                alert(result.error.message);
                setProcessingPaymentId(null);
            }

        } catch (error) {
            console.error("Payment Error:", error);
            alert("An unexpected error occurred during payment.");
            setProcessingPaymentId(null);
        }
    };

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
                                            <div className="flex justify-end space-x-2">
                                                {inv.status === 'Overdue' && (
                                                    <Button variant="ghost" size="icon" title="Send Reminder" className="text-amber-600 hover:text-amber-700 hover:bg-amber-50">
                                                        <Mail className="h-4 w-4" />
                                                    </Button>
                                                )}
                                                <DropdownMenu>
                                                    <DropdownMenuTrigger asChild>
                                                        <Button variant="ghost" size="icon">
                                                            <MoreHorizontal className="h-4 w-4" />
                                                        </Button>
                                                    </DropdownMenuTrigger>
                                                    <DropdownMenuContent align="end">
                                                        <DropdownMenuLabel>Actions</DropdownMenuLabel>
                                                        <DropdownMenuItem onClick={() => alert(`Viewing ${inv.id}`)}>
                                                            View Details
                                                        </DropdownMenuItem>
                                                        <DropdownMenuSeparator />
                                                        {inv.status === 'Pending' && (
                                                            <DropdownMenuItem
                                                                onClick={() => handlePayment(inv)}
                                                                className="text-primary focus:text-primary font-medium"
                                                                disabled={processingPaymentId === inv.id}
                                                            >
                                                                {processingPaymentId === inv.id ? (
                                                                    <><Loader2 className="mr-2 h-4 w-4 animate-spin" /> Processing...</>
                                                                ) : (
                                                                    <><CreditCard className="mr-2 h-4 w-4" /> Pay Now</>
                                                                )}
                                                            </DropdownMenuItem>
                                                        )}
                                                        <DropdownMenuItem onClick={() => window.print()}>
                                                            Download PDF
                                                        </DropdownMenuItem>
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
