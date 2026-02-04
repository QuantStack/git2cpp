import { delay, Shell } from '@jupyterlite/cockle';
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
    shellRun: (cmd: string, input: string | undefined | null) => shellRun(shell, output, cmd, input)
  };

  (window as any).cockle = cockle;

  // Add div to indicate setup complete which can be awaited at start of tests.
  const div = document.createElement("div");
  div.id = 'loaded';
  div.innerHTML = 'Loaded';
  document.body.appendChild(div);
}

async function shellRun(
  shell: Shell,
  output: MockTerminalOutput,
  cmd: string,
  input: string | undefined | null
): Promise<IReturn> {
  // Keep stdout and stderr separate by outputting stdout to terminal and stderr to temporary file,
  // then read the temporary file to get stderr to return.
  // There are issues here with use of \n and \r\n at ends of lines.
  output.clear();
  let cmdLine = cmd + ' 2> /drive/.errtmp' + '\r';

  if (input !== undefined && input !== null) {
    async function delayThenStdin(): Promise<void> {
      const chars = input! + '\x04';  // EOT
      await delay(100);
      for (const char of chars) {
        await shell.input(char);
        await delay(10);
      }
    }
    await Promise.all([shell.input(cmdLine), delayThenStdin()]);
  } else {
    await shell.input(cmdLine);
  }

  const stdout = stripOutput(output.textAndClear(), cmdLine);
  const returncode = await shell.exitCode();

  // Read stderr from .errtmp file.
  cmdLine = 'cat /drive/.errtmp\r';
  await shell.input(cmdLine);
  const stderr = stripOutput(output.textAndClear(), cmdLine);

  // Delete temporary file.
  cmdLine = 'rm /drive/.errtmp\r';
  await shell.input(cmdLine);
  output.clear();

  // Display in browser in new div.
  const div = document.createElement("div");
  let content = `> ${cmd}<br><span>returncode:</span> ${returncode}`
  if (stdout.length > 0) {
    content += `<br><span>stdout:</span> ${stdout.trim().replace(/\n/g, "<br>")}`;
  }
  if (stderr.length > 0) {
    content += `<br><span>stderr:</span> ${stderr.trim().replace(/\n/g, "<br>")}`;
  }
  div.innerHTML = content;
  document.body.appendChild(div);
  div.scrollIntoView();

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
