import { PrismaClient } from '@prisma/client';

const prisma = new PrismaClient();

async function main() {
    console.log(`Start seeding ...`);

    // 1. Seed Users
    const admin = await prisma.user.upsert({
        where: { username: 'admin' },
        update: {},
        create: {
            username: 'admin',
            password: 'admin123',
            role: 'ADMIN',
        },
    });

    const manager = await prisma.user.upsert({
        where: { username: 'manager' },
        update: {},
        create: {
            username: 'manager',
            password: 'manager123',
            role: 'MANAGER',
        },
    });

    const viewer = await prisma.user.upsert({
        where: { username: 'viewer' },
        update: {},
        create: {
            username: 'viewer',
            password: 'readonly',
            role: 'VIEWER',
        },
    });

    // 2. Seed Customers
    const customer1 = await prisma.customer.create({
        data: {
            id: 'CUST-001',
            name: 'Acme Corporation',
            email: 'billing@acme.inc',
            tier: 'Enterprise',
            creditScore: 780,
            lifetimeValue: 45000.00,
            status: 'Active',
            joined: new Date('2023-01-15'),
        },
    });

    const customer2 = await prisma.customer.create({
        data: {
            id: 'CUST-002',
            name: 'Stark Industries',
            email: 'tony@stark.com',
            tier: 'Gold',
            creditScore: 810,
            lifetimeValue: 125000.00,
            status: 'Active',
            joined: new Date('2022-11-04'),
        },
    });

    // 3. Seed Invoices
    const inv1 = await prisma.invoice.create({
        data: {
            id: 'INV-2024-1001',
            customerId: customer1.id,
            amount: 4500.00,
            dueDate: new Date('2024-03-15'),
            status: 'Paid',
            type: 'Recurring',
        }
    });

    const inv2 = await prisma.invoice.create({
        data: {
            id: 'INV-2024-1002',
            customerId: customer2.id,
            amount: 12500.00,
            dueDate: new Date('2024-03-10'),
            status: 'Overdue',
            type: 'One-Time',
        }
    });

    // 4. Seed Payments
    await prisma.payment.create({
        data: {
            id: 'PAY-8923',
            invoiceId: inv1.id,
            method: 'Credit Card',
            amount: 4500.00,
            status: 'Completed',
            date: new Date('2024-03-15T14:30:00Z'),
        }
    });

    // 5. Seed Audit Logs
    await prisma.auditLog.create({
        data: {
            user: 'admin',
            action: 'CREATE',
            entity: 'Invoice',
            details: 'Created INV-2024-1001 for Acme Corp',
        }
    });

    console.log(`Seeding finished.`);
}

main()
    .then(async () => {
        await prisma.$disconnect()
    })
    .catch(async (e) => {
        console.error(e)
        await prisma.$disconnect()
        process.exit(1)
    })
