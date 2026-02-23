import express from 'express';
import cors from 'cors';
import { PrismaClient } from '@prisma/client';
import Razorpay from 'razorpay';
import crypto from 'crypto';
import dotenv from 'dotenv';
dotenv.config();

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
        } else {
            res.status(401).json({ success: false, message: 'Invalid credentials' });
        }
    } catch (error) {
        res.status(500).json({ success: false, error: 'Database error' });
    }
});

// Signup API
app.post('/api/signup', async (req, res) => {
    const { username, email, password } = req.body;
    try {
        const existing = await prisma.user.findFirst({
            where: { OR: [{ username }, { email: email || 'invalid_email' }] }
        });
        if (existing) {
            return res.status(400).json({ success: false, message: 'Username or email already exists' });
        }
        const user = await prisma.user.create({
            data: { username, email, password, role: 'VIEWER' }
        });
        const { password: _, ...userWithoutPassword } = user;
        res.json({ success: true, token: 'mock-jwt-token-newuser', user: userWithoutPassword });
    } catch (error) {
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

// Users API (Admin Only typically)
app.get('/api/users', async (req, res) => {
    try {
        const users = await prisma.user.findMany({
            select: { id: true, username: true, role: true, status: true, lastLogin: true, createdAt: true }
        });
        res.json(users);
    } catch (error) {
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
    } catch (error) {
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

// Payments API
app.get('/api/payments', async (req, res) => {
    try {
        const payments = await prisma.payment.findMany({
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

// Audit Logs API
app.get('/api/audit', async (req, res) => {
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

app.listen(PORT, () => {
    console.log(`[Node] Billing Dashboard API running on http://localhost:${PORT}`);
});
