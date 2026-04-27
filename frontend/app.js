const API_URL = 'http://localhost:8080/api';
let currentAction = null;

function showMessage(msg, type) {
    const toast = document.createElement('div');
    toast.className = `message-toast ${type}`;
    toast.innerHTML = `<i class="fas ${type === 'success' ? 'fa-check-circle' : 'fa-exclamation-circle'}"></i> ${msg}`;
    document.body.appendChild(toast);
    setTimeout(() => toast.remove(), 3000);
}

function updateTime() {
    const now = new Date();
    const timeStr = now.toLocaleTimeString('en-US', { hour: '2-digit', minute: '2-digit', second: '2-digit' });
    const dateStr = now.toLocaleDateString('en-US', { month: 'short', day: 'numeric' });
    const timeElement = document.getElementById('currentTime');
    if (timeElement) timeElement.textContent = `${dateStr} ${timeStr}`;
}
setInterval(updateTime, 1000);
updateTime();

function showPage(pageName) {
    document.querySelectorAll('.page').forEach(page => page.classList.remove('active'));
    const targetPage = document.getElementById(`${pageName}Page`);
    if (targetPage) targetPage.classList.add('active');
    
    document.querySelectorAll('.nav-links a').forEach(link => link.classList.remove('active'));
    if (pageName === 'home') {
        const firstLink = document.querySelector('.nav-links a:first-child');
        if (firstLink) firstLink.classList.add('active');
        updateHomeStats();
    } else if (pageName === 'accounts') {
        const lastLink = document.querySelector('.nav-links a:last-child');
        if (lastLink) lastLink.classList.add('active');
        loadAllAccounts();
    }
}

function goBack() {
    showPage('home');
}

async function updateHomeStats() {
    try {
        const response = await fetch(`${API_URL}/accounts`);
        const data = await response.json();
        
        const totalAccSpan = document.getElementById('totalAccountsHome');
        const totalBalSpan = document.getElementById('totalBalanceHome');
        
        if (data.accounts && data.accounts.length > 0) {
            let totalBalance = 0;
            data.accounts.forEach(acc => totalBalance += acc.balance);
            if (totalAccSpan) totalAccSpan.textContent = data.accounts.length;
            if (totalBalSpan) totalBalSpan.textContent = `₹${totalBalance.toLocaleString()}`;
        } else {
            if (totalAccSpan) totalAccSpan.textContent = '0';
            if (totalBalSpan) totalBalSpan.textContent = '₹0';
        }
    } catch (error) {
        console.error('Error:', error);
    }
}

function showAction(action) {
    currentAction = action;
    const titles = {
        create: 'Open New Account',
        deposit: 'Deposit Money',
        withdraw: 'Withdraw Money',
        balance: 'Check Balance',
        delete: 'Delete Account',
        history: 'Transaction History'
    };
    
    let html = `<div class="form-card">
        <h2><i class="fas ${action === 'create' ? 'fa-user-plus' : action === 'deposit' ? 'fa-money-bill-wave' : action === 'withdraw' ? 'fa-hand-holding-usd' : action === 'balance' ? 'fa-chart-line' : action === 'delete' ? 'fa-trash-alt' : 'fa-history'}"></i> ${titles[action]}</h2>
        <form id="actionForm">`;
    
    if (action === 'create') {
        html += `
            <div class="form-group">
                <label><i class="fas fa-user"></i> Full Name *</label>
                <input type="text" id="name" required placeholder="Enter full name">
            </div>
            <div class="form-group">
                <label><i class="fas fa-id-card"></i> CNIC Number *</label>
                <input type="text" id="cnic" required placeholder="12345-6789012-3">
            </div>
            <div class="form-group">
                <label><i class="fas fa-phone"></i> Mobile Number</label>
                <input type="tel" id="mobile" placeholder="03001234567">
            </div>
            <div class="form-group">
                <label><i class="fas fa-envelope"></i> Email Address</label>
                <input type="email" id="email" placeholder="example@email.com">
            </div>
            <div class="form-group">
                <label><i class="fas fa-building"></i> Account Type</label>
                <select id="accType">
                    <option value="Savings">Savings Account</option>
                    <option value="Current">Current Account</option>
                    <option value="Premium">Premium Account</option>
                </select>
            </div>
            <div class="form-group">
                <label><i class="fas fa-rupee-sign"></i> Initial Deposit</label>
                <input type="number" id="deposit" step="0.01" value="0">
            </div>`;
    } else {
        html += `
            <div class="form-group">
                <label><i class="fas fa-hashtag"></i> Account Number</label>
                <input type="number" id="accountNo" required placeholder="Enter account number">
            </div>`;
        
        if (action === 'delete') {
            html += `
            <div class="form-group">
                <label><i class="fas fa-exclamation-triangle"></i> Confirm Deletion</label>
                <input type="text" id="confirm" required placeholder='Type "DELETE" to confirm'>
            </div>`;
        } else if (action !== 'balance' && action !== 'history') {
            html += `
            <div class="form-group">
                <label><i class="fas fa-rupee-sign"></i> Amount</label>
                <input type="number" id="amount" step="0.01" required placeholder="Enter amount">
            </div>`;
        }
    }
    
    html += `<button type="submit" class="btn-submit"><i class="fas fa-check"></i> ${titles[action]}</button>
        </form>
    </div>`;
    
    const actionContent = document.getElementById('actionContent');
    if (actionContent) {
        actionContent.innerHTML = html;
        const form = document.getElementById('actionForm');
        if (form) form.addEventListener('submit', handleFormSubmit);
    }
    showPage('action');
}

async function handleFormSubmit(e) {
    e.preventDefault();
    
    if (currentAction === 'create') {
        const name = document.getElementById('name')?.value;
        const cnic = document.getElementById('cnic')?.value;
        const mobile = document.getElementById('mobile')?.value || '';
        const email = document.getElementById('email')?.value || '';
        const accType = document.getElementById('accType')?.value || 'Savings';
        const deposit = parseFloat(document.getElementById('deposit')?.value) || 0;
        
        if (!name || !cnic) {
            showMessage('❌ Name and CNIC are required!', 'error');
            return;
        }
        
        try {
            const response = await fetch(`${API_URL}/accounts`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ name, cnic, mobile, email, accType, deposit })
            });
            const result = await response.json();
            
            if (result.success) {
                showMessage(`✅ Account created! Account No: ${result.accountNo}`, 'success');
                goBack();
                updateHomeStats();
            } else {
                showMessage(result.message || '❌ Error creating account', 'error');
            }
        } catch (error) {
            showMessage('❌ Cannot connect to backend! Make sure backend.exe is running', 'error');
        }
    }
    else if (currentAction === 'history') {
        const accountNo = parseInt(document.getElementById('accountNo')?.value);
        
        if (!accountNo) {
            showMessage('❌ Account number required', 'error');
            return;
        }
        
        try {
            const response = await fetch(`${API_URL}/history?account=${accountNo}`);
            const data = await response.json();
            
            if (data.transactions && data.transactions.length > 0) {
                let historyHtml = '<div class="form-card" style="max-width:800px;"><h2><i class="fas fa-history"></i> Transaction History</h2><div class="history-list">';
                data.transactions.forEach(t => {
                    let icon = t.type === 'Deposit' ? 'fa-arrow-down' : (t.type === 'Withdraw' ? 'fa-arrow-up' : 'fa-clock');
                    let color = t.type === 'Deposit' ? '#4caf50' : (t.type === 'Withdraw' ? '#f44336' : '#ffd700');
                    historyHtml += `
                        <div class="history-item">
                            <i class="fas ${icon}" style="color:${color}"></i>
                            <div class="history-details">
                                <strong>${t.type}</strong>
                                <small>${t.date}</small>
                            </div>
                            <div class="history-amount" style="color:${color}">
                                ${t.type === 'Deposit' ? '+' : (t.type === 'Withdraw' ? '-' : '')} ₹${t.amount}
                            </div>
                            <div class="history-balance">Balance: ₹${t.balanceAfter}</div>
                        </div>
                    `;
                });
                historyHtml += '</div><button type="button" class="back-btn" onclick="goBack()" style="margin-top:20px;">Close</button></div>';
                document.getElementById('actionContent').innerHTML = historyHtml;
            } else {
                showMessage('📭 No transaction history found', 'error');
                goBack();
            }
        } catch (error) {
            showMessage('❌ Cannot connect to backend!', 'error');
        }
    }
    else if (currentAction === 'delete') {
        const accountNo = parseInt(document.getElementById('accountNo')?.value);
        const confirm = document.getElementById('confirm')?.value;
        
        if (!accountNo) {
            showMessage('❌ Account number required', 'error');
            return;
        }
        
        if (confirm !== 'DELETE') {
            showMessage('❌ Type "DELETE" to confirm', 'error');
            return;
        }
        
        try {
            const response = await fetch(`${API_URL}/delete`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ accountNo })
            });
            const result = await response.json();
            
            if (result.success) {
                showMessage(`✅ Account ${accountNo} closed`, 'success');
                goBack();
                updateHomeStats();
            } else {
                showMessage(result.message, 'error');
            }
        } catch (error) {
            showMessage('❌ Cannot connect to backend!', 'error');
        }
    }
    else if (currentAction === 'deposit') {
        const accountNo = parseInt(document.getElementById('accountNo')?.value);
        const amount = parseFloat(document.getElementById('amount')?.value);
        
        if (!accountNo || !amount) {
            showMessage('❌ Account and amount required', 'error');
            return;
        }
        
        try {
            const response = await fetch(`${API_URL}/deposit`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ accountNo, amount })
            });
            const result = await response.json();
            
            if (result.success) {
                showMessage(`✅ Deposited ₹${amount}. New Balance: ₹${result.newBalance}`, 'success');
                goBack();
                updateHomeStats();
            } else {
                showMessage(result.message, 'error');
            }
        } catch (error) {
            showMessage('❌ Cannot connect to backend!', 'error');
        }
    }
    else if (currentAction === 'withdraw') {
        const accountNo = parseInt(document.getElementById('accountNo')?.value);
        const amount = parseFloat(document.getElementById('amount')?.value);
        
        if (!accountNo || !amount) {
            showMessage('❌ Account and amount required', 'error');
            return;
        }
        
        try {
            const response = await fetch(`${API_URL}/withdraw`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ accountNo, amount })
            });
            const result = await response.json();
            
            if (result.success) {
                showMessage(`✅ Withdrawn ₹${amount}. New Balance: ₹${result.newBalance}`, 'success');
                goBack();
                updateHomeStats();
            } else {
                showMessage(result.message, 'error');
            }
        } catch (error) {
            showMessage('❌ Cannot connect to backend!', 'error');
        }
    }
    else if (currentAction === 'balance') {
        const accountNo = parseInt(document.getElementById('accountNo')?.value);
        
        if (!accountNo) {
            showMessage('❌ Account number required', 'error');
            return;
        }
        
        try {
            const response = await fetch(`${API_URL}/balance?account=${accountNo}`);
            const result = await response.json();
            
            if (result.success) {
                showMessage(`💰 ${result.name} | Balance: ₹${result.balance}`, 'success');
            } else {
                showMessage(result.message, 'error');
            }
        } catch (error) {
            showMessage('❌ Cannot connect to backend!', 'error');
        }
    }
}

async function loadAllAccounts() {
    const container = document.getElementById('allAccountsList');
    if (!container) return;
    
    container.innerHTML = '<div class="loading"><i class="fas fa-spinner"></i> Loading...</div>';
    
    try {
        const response = await fetch(`${API_URL}/accounts`);
        const data = await response.json();
        
        if (data.accounts && data.accounts.length > 0) {
            container.innerHTML = data.accounts.map(acc => `
                <div class="account-card">
                    <h3><i class="fas fa-credit-card"></i> Account #${acc.accountNo}</h3>
                    <div class="detail"><span>Name:</span><span>${acc.name}</span></div>
                    <div class="detail"><span>CNIC:</span><span>${acc.cnic || 'N/A'}</span></div>
                    <div class="detail"><span>Mobile:</span><span>${acc.mobile || 'N/A'}</span></div>
                    <div class="detail"><span>Email:</span><span>${acc.email || 'N/A'}</span></div>
                    <div class="detail"><span>Type:</span><span>${acc.accType || 'Savings'}</span></div>
                    <div class="detail"><span>Balance:</span><span class="balance-amount">₹${acc.balance}</span></div>
                </div>
            `).join('');
        } else {
            container.innerHTML = '<div class="loading">📭 No accounts found</div>';
        }
    } catch (error) {
        container.innerHTML = '<div class="loading">⚠️ Cannot connect to backend! Make sure backend.exe is running on port 8080</div>';
    }
}

document.addEventListener('DOMContentLoaded', () => {
    updateHomeStats();
});