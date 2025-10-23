import { Shell } from '@jupyterlite/cockle';
import { MockTerminalOutput } from './utils';

interface IReturn {
  returncode: number;
  stdout: string;
  stderr: string;
}

async function setup() {
  const baseUrl = 'http://localhost:8000/';
  const output = new MockTerminalOutput();

  const shell = new Shell({
    baseUrl,
    wasmBaseUrl: baseUrl,
    outputCallback: output.callback,
    color: false
  });
  await shell.start();

  const cockle = {
    shell,
    shellRun: (cmd: string[]) => shellRun(shell, output, cmd)
  };

  (window as any).cockle = cockle;

  // Add div to indicate setup complete which can be awaited at start of tests.
  const div = document.createElement("div");
  div.id = 'loaded';
  div.innerHTML = 'loaded';
  document.body.appendChild(div);
}

async function shellRun(
  shell: Shell,
  output: MockTerminalOutput,
  cmd: string[]
): Promise<IReturn> {
  // Keep stdout and stderr separate by outputting stdout to temporary file and stderr to terminal,
  // then read the temporary file to get stdout to return.
  output.clear();
  let cmdLine = cmd.join(' ') + '> .outtmp' + '\r';
  await shell.input(cmdLine);

  const stderr = stripOutput(output.textAndClear(), cmdLine);
  const returncode = await shell.exitCode();

  // Read stdout from .outtmp file.
  cmdLine = 'cat .outtmp\r';
  await shell.input(cmdLine);
  const stdout = stripOutput(output.textAndClear(), cmdLine);

  // Delete temporary file.
  cmdLine = 'rm .outtmp\r';
  await shell.input(cmdLine);
  output.clear();

  return { returncode, stdout, stderr };
}

function stripOutput(output: string, cmdLine: string): string {
  // Remove submitted command line at start.
  let ret = output.slice(cmdLine.length);

  // Remove new prompt at end.
  const index = ret.lastIndexOf('\n');
  if (index >= 0) {
    ret = ret.slice(0, index + 1);
  } else {
    ret = '';
  }

  return ret;
}

setup();
