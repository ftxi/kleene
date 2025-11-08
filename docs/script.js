
// JavaScript interface with the WASM module (Mostly AI-generated)

let moduleReady = false;
let run_program_wasm = null;

// Wait for the Emscripten module to be ready
Module.onRuntimeInitialized = function() {
    // Wrap the C function for easier calling
    run_program_wasm = Module.cwrap('run_program', 'string', ['string', 'string', 'string']);
    moduleReady = true;
    console.log('WASM module loaded successfully');
};

// Wrapper function that calls the WASM function
function run_program(code, entry, input) {
    if (!moduleReady || !run_program_wasm) {
        throw new Error('WASM module not yet loaded');
    }
    return run_program_wasm(code, entry, input);
}

// Event listener for the Run button
document.getElementById('runBtn').addEventListener('click', function() {
    const code = document.getElementById('code').value;
    const entry = document.getElementById('entry').value;
    const input = document.getElementById('input').value;
    const outputBox = document.getElementById('output');
    const runBtn = document.getElementById('runBtn');

    // Validate entry (no whitespaces)
    if (entry && entry.length !== 0 && !/^\S+$/.test(entry)) {
        outputBox.textContent = 'Error: Entry must be an identifer (whitespaces not allowed)';
        outputBox.className = 'output-box error';
        return;
    }

    // Validate input (integers >= 0)
    if (input.trim()) {
        const numbers = input.trim().split(/\s+/);
        for (let num of numbers) {
            if (!/^\d+$/.test(num)) {
                outputBox.textContent = `Error: Input must contain only integers >= 0. Invalid value: "${num}"`;
                outputBox.className = 'output-box error';
                return;
            }
        }
    }

    // Disable button during execution
    runBtn.disabled = true;
    outputBox.textContent = 'Running...';
    outputBox.className = 'output-box';

    // Simulate async execution
    setTimeout(() => {
        try {
            const result = run_program(code, entry, input);
            outputBox.textContent = result;
            if (/^\d+$/.test(result)) {
                outputBox.className = 'output-box success';
            } else {
                outputBox.className = 'output-box warning';
            }
        } catch (error) {
            outputBox.textContent = `Client-side error: ${error.message}`;
            outputBox.className = 'output-box error';
        } finally {
            runBtn.disabled = false;
        }
    }, 100);
});

// Allow Enter key in single-line inputs to trigger run
document.getElementById('entry').addEventListener('keypress', function(e) {
    if (e.key === 'Enter') {
        document.getElementById('runBtn').click();
    }
});

document.getElementById('input').addEventListener('keypress', function(e) {
    if (e.key === 'Enter') {
        document.getElementById('runBtn').click();
    }
});


const examplesBtn = document.getElementById('loadExamplesBtn');
const examplesSelect = document.getElementById('examplesSelect');
const codeArea = document.getElementById('code');

// Fetch the list of files (you can generate a JSON file with filenames from ./ex/)
async function fetchExamples() {
    const res = await fetch('./ex/examples.json'); // JSON: ["file1.txt", "file2.txt", ...]
    const files = await res.json();
    
    // Populate select
    examplesSelect.innerHTML = '<option value="">--- Select example ---</option>';
    files.forEach(file => {
        const opt = document.createElement('option');
        opt.value = file;
        opt.textContent = file.slice(0,-3);
        examplesSelect.appendChild(opt);
    });
    examplesSelect.style.display = 'inline';
}

fetchExamples();

// When user selects a file
examplesBtn.addEventListener('click', async () => {
    const file = examplesSelect.value;
    if (!file) return;
    const res = await fetch(`./ex/${file}`);
    const content = await res.text();
    codeArea.value = content;
});
