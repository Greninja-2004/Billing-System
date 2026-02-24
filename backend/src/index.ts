import express from 'express';
import cors from 'cors';
import { PrismaClient } from '@prisma/client';
import Razorpay from 'razorpay';
import crypto from 'crypto';
import jwt from 'jsonwebtoken';
import dotenv from 'dotenv';
dotenv.config();

const JWT_SECRET = process.env.JWT_SECRET || 'billing_pro_jwt_secret_change_in_prod';

// ── Auth Middleware ───────────────────────────────────────────────────────────
function requireAuth(req: any, res: any, next: any) {
    const authHeader = req.headers['authorization'];
    const token = authHeader && authHeader.startsWith('Bearer ') ? authHeader.slice(7) : null;
    if (!token) return res.status(401).json({ error: 'Authentication required' });
    try {
        req.user = jwt.verify(token, JWT_SECRET) as any;
        next();
    } catch {
        return res.status(403).json({ error: 'Invalid or expired token' });
    }
}

function requireAdmin(req: any, res: any, next: any) {
    if (!req.user || (req.user.role !== 'ADMIN' && req.user.role !== 'MANAGER')) {
        return res.status(403).json({ error: 'Access denied: Admin/Manager only' });
    }
    next();
}

const razorpay = new Razorpay({
    key_id: process.env.RAZORPAY_KEY_ID || 'rzp_test_SJds2D7mndHYaD',
    key_secret: process.env.RAZORPAY_KEY_SECRET || '5ExZZIM32zSddaXSaGdmDUbW',
});

const app = express();
const PORT = process.env.PORT || 3001;
const prisma = new PrismaClient();

// Allow production Vercel frontend or local Vite frontend
const allowedOrigins = [
    'http://localhost:5173',
    'http://localhost:5174',
    'http://localhost:5175',
    process.env.FRONTEND_URL || ''
].filter(Boolean);

app.use(cors({ origin: allowedOrigins }));
app.use(express.json());

// Basic health check endpoint
app.get('/api/health', (req, res) => {
    res.json({ status: 'ok', database: 'connected' });
});

// Login API — issues a real JWT
app.post('/api/login', async (req, res) => {
    const { username, password } = req.body;
    try {
        const user = await prisma.user.findUnique({ where: { username } });
        if (user && user.password === password) {
            await prisma.user.update({ where: { id: user.id }, data: { lastLogin: new Date() } });
            const { password: _, ...userWithoutPassword } = user;
            const token = jwt.sign(
                { id: user.id, username: user.username, role: user.role, email: user.email },
                JWT_SECRET,
                { expiresIn: '7d' }
            );
            res.json({ success: true, token, user: userWithoutPassword });
        } else {
            res.status(401).json({ success: false, message: 'Invalid credentials' });
        }
    } catch (error) {
        res.status(500).json({ success: false, error: 'Database error' });
    }
});

// Signup API — creates VIEWER user + linked Customer record
app.post('/api/signup', async (req, res) => {
    const { username, email, password } = req.body;
    try {
        const existing = await prisma.user.findFirst({
            where: { OR: [{ username }, { email: email || '_no_email_' }] }
        });
        if (existing) return res.status(400).json({ success: false, message: 'Username or email already exists' });

        const user = await prisma.user.create({
            data: { username, email, password, role: 'VIEWER' }
        });

        // Auto-create a linked Customer record for this new user
        await prisma.customer.create({
            data: {
                id: `CUST-${user.id.substring(0, 8).toUpperCase()}`,
                name: username,
                email: email || `${username}@billing.local`,
                tier: 'Standard',
                creditScore: 700,
                lifetimeValue: 0,
                status: 'Active',
                joined: new Date(),
                userId: user.id,
            }
        });

        const { password: _, ...userWithoutPassword } = user;
        const token = jwt.sign(
            { id: user.id, username: user.username, role: user.role, email: user.email },
            JWT_SECRET,
            { expiresIn: '7d' }
        );
        res.json({ success: true, token, user: userWithoutPassword });
    } catch (error: any) {
        console.error('Signup error:', error);
        res.status(500).json({ success: false, error: 'Database error' });
    }
});

// OAuth Initialization
app.get('/api/auth/:provider', (req, res) => {
    const { provider } = req.params;
    // Real implementation would redirect to Google/GitHub/Apple authorization URL here using process.env.CLIENT_ID

    // For local testing, we show a simulated OAuth provider consent screen
    res.send(`
        <html>
            <body style="font-family: sans-serif; display: flex; align-items:center; justify-content:center; height: 100vh; background: #09090b; color: #fff;">
                <div style="background: #18181b; padding: 40px; border-radius: 12px; box-shadow: 0 4px 20px rgba(0,0,0,0.5); text-align: center; border: 1px solid #27272a;">
                    <h2 style="margin-top: 0; text-transform: capitalize;">Authorize with ${provider}</h2>
                    <p style="color: #a1a1aa; max-width: 300px; margin: 0 auto 30px;">This is a local simulated OAuth flow. In production, this would be the actual ${provider} login screen.</p>
                    <a href="/api/auth/${provider}/callback?code=simulated_${provider}_code" style="display:inline-block; padding: 12px 24px; background: #c5f82a; color: black; text-decoration: none; border-radius: 8px; font-weight: 600;">Approve & Continue</a>
                </div>
            </body>
        </html>
    `);
});

// OAuth Callback
app.get('/api/auth/:provider/callback', async (req, res) => {
    const { provider } = req.params;
    const { code } = req.query;

    if (!code) return res.status(400).send('No code provided');

    try {
        // Real implementation: exchange 'code' for access token, then fetch user profile from provider API.
        const providerId = `mock_${provider}_${Math.floor(Math.random() * 100000)}`;
        const email = `user@${provider}.local`;
        const username = `${provider}_user_${Math.floor(Math.random() * 1000)}`;

        let user = await prisma.user.findFirst({
            where: {
                OR: [
                    { googleId: provider === 'google' ? providerId : 'IGNORE' },
                    { githubId: provider === 'github' ? providerId : 'IGNORE' },
                    { appleId: provider === 'apple' ? providerId : 'IGNORE' },
                    { email } // Link account if email matches
                ]
            }
        });

        if (!user) {
            user = await prisma.user.create({
                data: {
                    username,
                    email,
                    googleId: provider === 'google' ? providerId : null,
                    githubId: provider === 'github' ? providerId : null,
                    appleId: provider === 'apple' ? providerId : null,
                    role: 'VIEWER'
                }
            });
        }

        const { password: _, ...userWithoutPassword } = user;
        const userObjEnc = encodeURIComponent(JSON.stringify(userWithoutPassword));

        // Redirect back to frontend with tokens
        res.redirect(`http://localhost:5173/login?social_token=mock_jwt_token&social_user=${userObjEnc}`);

    } catch (error) {
        console.error('OAuth Callback Error:', error);
        res.redirect('http://localhost:5173/login?error=oauth_failed');
    }
});

// Users API (ADMIN only)
app.get('/api/users', requireAuth, requireAdmin, async (req, res) => {
    try {
        const users = await prisma.user.findMany({
            select: { id: true, username: true, role: true, status: true, lastLogin: true, createdAt: true }
        });
        res.json(users);
    } catch (error) {
        res.status(500).json({ error: 'Failed to fetch users' });
    }
});

// Create Customer (linked to the authenticated user)
app.post('/api/customers', requireAuth, async (req: any, res) => {
    try {
        const { name, email, tier, phone } = req.body;
        if (!name || !email) return res.status(400).json({ error: 'Name and email are required' });

        const customer = await prisma.customer.create({
            data: {
                id: `CUST-${Date.now().toString(36).toUpperCase()}`,
                name,
                email,
                tier: tier || 'Standard',
                creditScore: 700,
                lifetimeValue: 0,
                status: 'Active',
                joined: new Date(),
                userId: req.user.id,
            }
        });
        res.json({ success: true, customer });
    } catch (error: any) {
        console.error('Create customer error:', error);
        if (error.code === 'P2002') return res.status(400).json({ error: 'Email already exists' });
        res.status(500).json({ error: 'Failed to create customer' });
    }
});

// Create Invoice (linked to one of the user's customers)
app.post('/api/invoices', requireAuth, async (req: any, res) => {
    try {
        const { customerId, amount, dueDate, type } = req.body;
        if (!customerId || !amount || !dueDate) return res.status(400).json({ error: 'customerId, amount, and dueDate are required' });

        // Ensure the customer belongs to this user (unless admin)
        const isAdmin = req.user.role === 'ADMIN' || req.user.role === 'MANAGER';
        const customer = await prisma.customer.findFirst({
            where: isAdmin ? { id: customerId } : { id: customerId, userId: req.user.id }
        });
        if (!customer) return res.status(404).json({ error: 'Customer not found or not yours' });

        const invoice = await prisma.invoice.create({
            data: {
                id: `INV-${Date.now().toString(36).toUpperCase()}`,
                customerId,
                amount: parseFloat(amount),
                dueDate: new Date(dueDate),
                status: 'Pending',
                type: type || 'One-Time',
            }
        });
        res.json({ success: true, invoice });
    } catch (error: any) {
        console.error('Create invoice error:', error);
        res.status(500).json({ error: 'Failed to create invoice' });
    }
});


// Customers API — ADMIN/MANAGER see all, VIEWER sees only their own
app.get('/api/customers', requireAuth, async (req: any, res) => {
    try {
        const isAdmin = req.user.role === 'ADMIN' || req.user.role === 'MANAGER';
        const customers = await prisma.customer.findMany({
            where: isAdmin ? {} : { userId: req.user.id },
            orderBy: { joined: 'desc' }
        });
        res.json(customers);
    } catch (error) {
        res.status(500).json({ error: 'Failed to fetch customers' });
    }
});

// Invoices API — ADMIN/MANAGER see all, VIEWER sees only their own
app.get('/api/invoices', requireAuth, async (req: any, res) => {
    try {
        const isAdmin = req.user.role === 'ADMIN' || req.user.role === 'MANAGER';
        const invoices = await prisma.invoice.findMany({
            where: isAdmin ? {} : ({ customer: { userId: String(req.user.id) } } as any),
            include: { customer: { select: { name: true } } },
            orderBy: { dueDate: 'desc' }
        });
        const formattedInvoices = invoices.map((i: any) => ({
            ...i,
            customerName: i.customer.name,
            due_date: i.dueDate.toISOString()
        }));
        res.json(formattedInvoices);
    } catch (error) {
        res.status(500).json({ error: 'Failed to fetch invoices' });
    }
});

// Payments API — ADMIN/MANAGER see all, VIEWER sees only their own
app.get('/api/payments', requireAuth, async (req: any, res) => {
    try {
        const isAdmin = req.user.role === 'ADMIN' || req.user.role === 'MANAGER';
        const payments = await prisma.payment.findMany({
            where: isAdmin ? {} : ({ invoice: { customer: { userId: String(req.user.id) } } } as any),
            include: { invoice: { select: { customer: { select: { name: true } } } } },
            orderBy: { date: 'desc' }
        });
        const formattedPayments = payments.map((p: any) => ({
            ...p,
            customerName: p.invoice.customer.name
        }));
        res.json(formattedPayments);
    } catch (error) {
        res.status(500).json({ error: 'Failed to fetch payments' });
    }
});

// Audit Logs API (ADMIN/MANAGER only)
app.get('/api/audit', requireAuth, requireAdmin, async (req, res) => {
    try {
        const logs = await prisma.auditLog.findMany({
            orderBy: { time: 'desc' }
        });
        res.json(logs);
    } catch (error) {
        res.status(500).json({ error: 'Failed to fetch audit logs' });
    }
});

// Razorpay: Create Order
app.post('/api/create-razorpay-order', async (req, res) => {
    try {
        const { invoiceId, amount, customerName } = req.body;

        // Razorpay expects amount in paisa (1 INR = 100 paisa)
        const amountInPaisa = Math.round(Number(amount) * 100);

        const order = await razorpay.orders.create({
            amount: amountInPaisa,
            currency: 'INR',
            receipt: `rcpt_${invoiceId}`.substring(0, 40),
            notes: { invoiceId, customerName },
        });

        res.json({
            orderId: order.id,
            amount: order.amount,
            currency: order.currency,
            keyId: process.env.RAZORPAY_KEY_ID || 'rzp_test_SJdYdtSCSObO2t',
        });
    } catch (error: any) {
        console.error('Razorpay Order Error:', error);
        res.status(500).json({ error: error.message || 'Failed to create order' });
    }
});

// Razorpay: Verify Payment Signature
app.post('/api/verify-razorpay-payment', async (req, res) => {
    try {
        const { razorpay_order_id, razorpay_payment_id, razorpay_signature } = req.body;
        const secret = process.env.RAZORPAY_KEY_SECRET || 'YOUR_KEY_SECRET_HERE';

        const hmac = crypto.createHmac('sha256', secret);
        hmac.update(`${razorpay_order_id}|${razorpay_payment_id}`);
        const generated_signature = hmac.digest('hex');

        if (generated_signature === razorpay_signature) {
            res.json({ success: true, message: 'Payment verified successfully' });
        } else {
            res.status(400).json({ success: false, message: 'Payment verification failed' });
        }
    } catch (error: any) {
        console.error('Razorpay Verify Error:', error);
        res.status(500).json({ error: error.message });
    }
});

// Dashboard Stats — ADMIN/MANAGER see all, VIEWER sees only their own
app.get('/api/dashboard/stats', requireAuth, async (req: any, res) => {
    try {
        const isAdmin = req.user.role === 'ADMIN' || req.user.role === 'MANAGER';
        const invoiceWhere: any = isAdmin ? {} : { customer: { userId: String(req.user.id) } };

        const [revenueResult, pendingCount, overdueCount, recentPayments] = await Promise.all([
            prisma.invoice.aggregate({
                where: { ...invoiceWhere, status: 'Paid' },
                _sum: { amount: true },
            }),
            prisma.invoice.count({ where: { ...invoiceWhere, status: 'Pending' } }),
            prisma.invoice.count({ where: { ...invoiceWhere, status: 'Overdue' } }),
            prisma.payment.findMany({
                where: isAdmin ? {} : ({ invoice: { customer: { userId: String(req.user.id) } } } as any),
                orderBy: { date: 'desc' },
                take: 5,
                include: { invoice: { select: { customer: { select: { name: true } } } } },
            }),
        ]);

        // Count active subscriptions (simulated from paid invoices)
        const activeSubs = await prisma.invoice.count({
            where: { ...invoiceWhere, type: 'Recurring', status: 'Paid' },
        });

        res.json({
            revenue: revenueResult._sum?.amount || 0,
            pendingInvoices: pendingCount,
            overdueAccounts: overdueCount,
            activeSubscriptions: activeSubs,
            recentPayments: recentPayments.map((p: any) => ({
                id: p.id,
                customerName: p.invoice?.customer?.name || 'Unknown',
                amount: p.amount,
                method: p.method,
                status: p.status,
                date: p.date,
            })),
        });
    } catch (error) {
        console.error('Dashboard stats error:', error);
        res.status(500).json({ error: 'Failed to fetch dashboard stats' });
    }
});

app.listen(PORT, () => {
    console.log(`[Node] Billing Dashboard API running on http://localhost:${PORT}`);
});
