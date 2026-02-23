"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
const express_1 = __importDefault(require("express"));
const cors_1 = __importDefault(require("cors"));
const client_1 = require("@prisma/client");
const stripe_1 = __importDefault(require("stripe"));
const stripe = new stripe_1.default('sk_test_51OzA...fakeTestKey...replaceWithRealLater', {
    apiVersion: '2023-10-16',
});
const app = (0, express_1.default)();
const PORT = process.env.PORT || 3001;
const prisma = new client_1.PrismaClient();
// Allow production Vercel frontend or local Vite frontend
const allowedOrigins = [
    'http://localhost:5173',
    'http://localhost:5174',
    'http://localhost:5175',
    process.env.FRONTEND_URL || ''
].filter(Boolean);
app.use((0, cors_1.default)({ origin: allowedOrigins }));
app.use(express_1.default.json());
// Basic health check endpoint
app.get('/api/health', (req, res) => {
    res.json({ status: 'ok', database: 'connected' });
});
// Authentication API
app.post('/api/login', async (req, res) => {
    const { username, password } = req.body;
    try {
        const user = await prisma.user.findUnique({ where: { username } });
        // In a real app, compare hashed passwords here!
        if (user && user.password === password) {
            // Update last login
            await prisma.user.update({
                where: { id: user.id },
                data: { lastLogin: new Date() }
            });
            const { password: _, ...userWithoutPassword } = user;
            res.json({ success: true, token: 'fake-jwt-token-bz9182', user: userWithoutPassword });
        }
        else {
            res.status(401).json({ success: false, message: 'Invalid credentials' });
        }
    }
    catch (error) {
        res.status(500).json({ success: false, error: 'Database error' });
    }
});
// Users API (Admin Only typically)
app.get('/api/users', async (req, res) => {
    try {
        const users = await prisma.user.findMany({
            select: { id: true, username: true, role: true, status: true, lastLogin: true, createdAt: true }
        });
        res.json(users);
    }
    catch (error) {
        res.status(500).json({ error: 'Failed to fetch users' });
    }
});
// Customers API
app.get('/api/customers', async (req, res) => {
    try {
        const customers = await prisma.customer.findMany({
            orderBy: { joined: 'desc' }
        });
        res.json(customers);
    }
    catch (error) {
        res.status(500).json({ error: 'Failed to fetch customers' });
    }
});
// Invoices API
app.get('/api/invoices', async (req, res) => {
    try {
        const invoices = await prisma.invoice.findMany({
            include: { customer: { select: { name: true } } },
            orderBy: { dueDate: 'desc' }
        });
        // Flatten the response slightly to match frontend expectations
        const formattedInvoices = invoices.map((i) => ({
            ...i,
            customerName: i.customer.name,
            due_date: i.dueDate.toISOString()
        }));
        res.json(formattedInvoices);
    }
    catch (error) {
        res.status(500).json({ error: 'Failed to fetch invoices' });
    }
});
// Payments API
app.get('/api/payments', async (req, res) => {
    try {
        const payments = await prisma.payment.findMany({
            include: { invoice: { select: { customer: { select: { name: true } } } } },
            orderBy: { date: 'desc' }
        });
        const formattedPayments = payments.map((p) => ({
            ...p,
            customerName: p.invoice.customer.name
        }));
        res.json(formattedPayments);
    }
    catch (error) {
        res.status(500).json({ error: 'Failed to fetch payments' });
    }
});
// Audit Logs API
app.get('/api/audit', async (req, res) => {
    try {
        const logs = await prisma.auditLog.findMany({
            orderBy: { time: 'desc' }
        });
        res.json(logs);
    }
    catch (error) {
        res.status(500).json({ error: 'Failed to fetch audit logs' });
    }
});
// Stripe Checkout API
app.post('/api/create-checkout-session', async (req, res) => {
    try {
        const { invoiceId, amount, customerName } = req.body;
        // In a real application, you would lookup the invoice in the database to verify the amount
        // const invoice = await prisma.invoice.findUnique({ where: { id: invoiceId } });
        const session = await stripe.checkout.sessions.create({
            payment_method_types: ['card'],
            line_items: [
                {
                    price_data: {
                        currency: 'usd',
                        product_data: {
                            name: `Invoice ${invoiceId}`,
                            description: `Payment for ${customerName}`,
                        },
                        unit_amount: Math.round(amount * 100), // Stripe expects cents
                    },
                    quantity: 1,
                },
            ],
            mode: 'payment',
            success_url: `http://localhost:5175/success?session_id={CHECKOUT_SESSION_ID}`,
            cancel_url: `http://localhost:5175/invoices`,
            metadata: {
                invoiceId: invoiceId,
            }
        });
        res.json({ id: session.id, url: session.url });
    }
    catch (error) {
        console.error("Stripe Error:", error);
        res.status(500).json({ error: error.message });
    }
});
app.listen(PORT, () => {
    console.log(`[Node] Billing Dashboard API running on http://localhost:${PORT}`);
});
//# sourceMappingURL=index.js.map