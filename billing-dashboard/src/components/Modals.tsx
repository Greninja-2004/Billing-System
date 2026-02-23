import { useState, useEffect } from 'react';
import { apiFetch } from '../utils/api';
import { Button } from './ui/button';
import { Input } from './ui/input';
import { X, Loader2, UserPlus, FileText } from 'lucide-react';

interface Customer {
    id: string;
    name: string;
    email: string;
}

// ─── Add Customer Modal ────────────────────────────────────────────────────────
interface AddCustomerModalProps {
    open: boolean;
    onClose: () => void;
    onSuccess: (customer: Customer) => void;
}

export function AddCustomerModal({ open, onClose, onSuccess }: AddCustomerModalProps) {
    const [name, setName] = useState('');
    const [email, setEmail] = useState('');
    const [tier, setTier] = useState('Standard');
    const [saving, setSaving] = useState(false);
    const [error, setError] = useState('');

    useEffect(() => {
        if (open) { setName(''); setEmail(''); setTier('Standard'); setError(''); }
    }, [open]);

    const handleSubmit = async (e: React.FormEvent) => {
        e.preventDefault();
        setSaving(true);
        setError('');
        try {
            const res = await apiFetch('/api/customers', {
                method: 'POST',
                body: JSON.stringify({ name, email, tier }),
            });
            const data = await res.json();
            if (!res.ok) { setError(data.error || 'Failed to create customer'); setSaving(false); return; }
            onSuccess(data.customer);
            onClose();
        } catch {
            setError('Network error. Please try again.');
        } finally {
            setSaving(false);
        }
    };

    if (!open) return null;
    return (
        <div className="fixed inset-0 z-50 flex items-center justify-center">
            <div className="absolute inset-0 bg-black/60 backdrop-blur-sm" onClick={onClose} />
            <div className="relative bg-card border border-border rounded-2xl shadow-2xl w-full max-w-md mx-4 p-6 z-10 animate-slide-up">
                <div className="flex items-center justify-between mb-5">
                    <div className="flex items-center gap-3">
                        <div className="p-2 bg-blue-500/10 rounded-xl"><UserPlus className="h-5 w-5 text-blue-500" /></div>
                        <div>
                            <h2 className="text-lg font-bold">Add Customer</h2>
                            <p className="text-xs text-muted-foreground">Create a new customer record</p>
                        </div>
                    </div>
                    <button onClick={onClose} className="text-muted-foreground hover:text-foreground"><X className="h-5 w-5" /></button>
                </div>
                {error && <div className="mb-4 p-3 rounded-lg bg-destructive/10 text-destructive text-sm">{error}</div>}
                <form onSubmit={handleSubmit} className="space-y-4">
                    <div>
                        <label className="text-sm font-medium mb-1 block">Full Name *</label>
                        <Input value={name} onChange={e => setName(e.target.value)} placeholder="Acme Corp" required />
                    </div>
                    <div>
                        <label className="text-sm font-medium mb-1 block">Email *</label>
                        <Input type="email" value={email} onChange={e => setEmail(e.target.value)} placeholder="billing@acme.com" required />
                    </div>
                    <div>
                        <label className="text-sm font-medium mb-1 block">Tier</label>
                        <select
                            value={tier} onChange={e => setTier(e.target.value)}
                            className="w-full h-9 rounded-md border border-input bg-background px-3 text-sm"
                        >
                            <option>Standard</option>
                            <option>Silver</option>
                            <option>Gold</option>
                            <option>Enterprise</option>
                        </select>
                    </div>
                    <div className="flex gap-2 pt-2">
                        <Button type="button" variant="outline" className="flex-1" onClick={onClose}>Cancel</Button>
                        <Button type="submit" className="flex-1" disabled={saving}>
                            {saving ? <><Loader2 className="h-4 w-4 mr-2 animate-spin" />Saving...</> : 'Create Customer'}
                        </Button>
                    </div>
                </form>
            </div>
        </div>
    );
}

// ─── New Invoice Modal ─────────────────────────────────────────────────────────
interface NewInvoiceModalProps {
    open: boolean;
    onClose: () => void;
    onSuccess: (invoice: any) => void;
}

export function NewInvoiceModal({ open, onClose, onSuccess }: NewInvoiceModalProps) {
    const [customers, setCustomers] = useState<Customer[]>([]);
    const [customerId, setCustomerId] = useState('');
    const [amount, setAmount] = useState('');
    const [type, setType] = useState('One-Time');
    const [saving, setSaving] = useState(false);
    const [error, setError] = useState('');

    // Default due date: 30 days from today
    const getDefaultDue = () => {
        const d = new Date();
        d.setDate(d.getDate() + 30);
        return d.toISOString().split('T')[0];
    };
    const [dueDate, setDueDate] = useState(getDefaultDue());

    useEffect(() => {
        if (open) {
            setAmount(''); setDueDate(getDefaultDue()); setType('One-Time'); setError('');
            apiFetch('/api/customers').then(r => r.json()).then(data => {
                const list = Array.isArray(data) ? data : [];
                setCustomers(list);
                if (list.length > 0) setCustomerId(list[0].id);
            });
        }
    }, [open]);

    const handleSubmit = async (e: React.FormEvent) => {
        e.preventDefault();
        if (!customerId) { setError('Please add a customer first before creating an invoice.'); return; }
        setSaving(true);
        setError('');
        try {
            const res = await apiFetch('/api/invoices', {
                method: 'POST',
                body: JSON.stringify({ customerId, amount, dueDate, type }),
            });
            const data = await res.json();
            if (!res.ok) { setError(data.error || 'Failed to create invoice'); setSaving(false); return; }
            onSuccess(data.invoice);
            onClose();
        } catch {
            setError('Network error. Please try again.');
        } finally {
            setSaving(false);
        }
    };

    if (!open) return null;

    // Tomorrow's date as default
    const tomorrow = new Date();
    tomorrow.setDate(tomorrow.getDate() + 30);
    const defaultDue = tomorrow.toISOString().split('T')[0];

    return (
        <div className="fixed inset-0 z-50 flex items-center justify-center">
            <div className="absolute inset-0 bg-black/60 backdrop-blur-sm" onClick={onClose} />
            <div className="relative bg-card border border-border rounded-2xl shadow-2xl w-full max-w-md mx-4 p-6 z-10 animate-slide-up">
                <div className="flex items-center justify-between mb-5">
                    <div className="flex items-center gap-3">
                        <div className="p-2 bg-emerald-500/10 rounded-xl"><FileText className="h-5 w-5 text-emerald-500" /></div>
                        <div>
                            <h2 className="text-lg font-bold">New Invoice</h2>
                            <p className="text-xs text-muted-foreground">Create an invoice for a customer</p>
                        </div>
                    </div>
                    <button onClick={onClose} className="text-muted-foreground hover:text-foreground"><X className="h-5 w-5" /></button>
                </div>
                {error && <div className="mb-4 p-3 rounded-lg bg-destructive/10 text-destructive text-sm">{error}</div>}
                <form onSubmit={handleSubmit} className="space-y-4">
                    <div>
                        <label className="text-sm font-medium mb-1 block">Customer *</label>
                        {customers.length === 0 ? (
                            <div className="text-sm text-muted-foreground p-3 bg-muted rounded-lg">
                                ⚠️ No customers yet. <strong>Add a customer first</strong> before creating an invoice.
                            </div>
                        ) : (
                            <select
                                value={customerId} onChange={e => setCustomerId(e.target.value)} required
                                className="w-full h-9 rounded-md border border-input bg-background px-3 text-sm"
                            >
                                {customers.map(c => <option key={c.id} value={c.id}>{c.name} ({c.email})</option>)}
                            </select>
                        )}
                    </div>
                    <div>
                        <label className="text-sm font-medium mb-1 block">Amount (₹) *</label>
                        <Input type="number" min="1" step="0.01" value={amount} onChange={e => setAmount(e.target.value)} placeholder="2500.00" required />
                    </div>
                    <div>
                        <label className="text-sm font-medium mb-1 block">Due Date *</label>
                        <Input type="date" value={dueDate || defaultDue} onChange={e => setDueDate(e.target.value)} required />
                    </div>
                    <div>
                        <label className="text-sm font-medium mb-1 block">Invoice Type</label>
                        <select value={type} onChange={e => setType(e.target.value)}
                            className="w-full h-9 rounded-md border border-input bg-background px-3 text-sm">
                            <option>One-Time</option>
                            <option>Recurring</option>
                            <option>Prorated</option>
                        </select>
                    </div>
                    <div className="flex gap-2 pt-2">
                        <Button type="button" variant="outline" className="flex-1" onClick={onClose}>Cancel</Button>
                        <Button type="submit" className="flex-1" disabled={saving || customers.length === 0}>
                            {saving ? <><Loader2 className="h-4 w-4 mr-2 animate-spin" />Creating...</> : 'Create Invoice'}
                        </Button>
                    </div>
                </form>
            </div>
        </div>
    );
}
