<?php
   @mkdir('instances');
   @mkdir('logs');
   @mkdir('tex');
   @mkdir('pdf');

   // program compilation
   $programs = [
      'exact_gurobi', 
      'exact_hungarian_1bad', 
      'exact_hungarian_allbads', 
      'exact_nodualupdate_1bad', 
      'exact_nodualupdate_allbads', 
      'exact_subcubic_1bad_double', 
      'exact_subcubic_1bad_mpfloat', 
      'exact_subcubic_1bad_novoronoi', 
      'exact_subcubic_allbads_double',
      'exact_subcubic_allbads_mpfloat',
      'exact_subcubic_allbads_novoronoi',
      'heuristic_nearestneighbor', 
      'heuristic_bestoftwo',
      'heuristic_greedystar', 
      'heuristic_greedystar_improved',
      '_instance_generator',
      '_verifier'
   ];
   foreach ($programs as $program) {
      echo "Compiling $program...\n";
      system("g++ -std=c++2b -O3 $program.cpp -lgurobi_c++ -lgurobi90 -Wno-return-type -o $program");
   }
   array_pop($programs); array_pop($programs);

   // instance generation
   foreach ([ 50, 100, 500, 1000, 2500, 5000 ] as $p) {
      foreach ([ [ $p / 2, $p / 2 ] ] as $partition) {
         list($a, $b) = $partition;
         foreach ([ 'R', 'P', 'E' ] as $c) {
            foreach ([ 0 ] as $s) {
               echo "Generating $a $b $c $s...\n";
               system("echo $a $b $c $s | ./_instance_generator > instances/{$a}_{$b}_{$c}_{$s}.in");
            }
         }
      }
   }
   $instances = array_map(fn($s) => pathinfo($s)['filename'], glob('instances/*'));
   natsort($instances);
   
   // instance solving and verification
   $worst = array_combine($programs, array_fill(0, count($programs), 1));
   $padding = max(array_map('strlen', $programs));
   $limit = 1800;

   foreach ($instances as $instance) {
      $results = [ ];
      echo "Solving {$instance}...\n";
      foreach ($programs as $program) {         
         echo "   $program", str_pad('', $padding - strlen($program) + 3, '.'), ' ';
         $t0 = microtime(true);
         system("timeout {$limit}s ./{$program} < instances/{$instance}.in > logs/{$instance}_{$program}.out", $status);
         $t1 = microtime(true);
         $time = $t1 - $t0;
         if ($time >= $limit || $status != 0) {
            echo ($time >= $limit ? 'TLE' : 'RTE'), "\n";
            unlink("logs/{$instance}_{$program}.out");
            continue;
         } else if (exec("cat instances/$instance.in logs/{$instance}_{$program}.out | ./_verifier") != '1') {
            echo "WA\n";
            continue;
         }
         $temp = file("logs/{$instance}_{$program}.out", FILE_IGNORE_NEW_LINES|FILE_SKIP_EMPTY_LINES);
         $results[$program] = (float)end($temp);
         $ratio = ($results['exact_gurobi'] == 0 ? 1 : $results[$program] / $results['exact_gurobi']);
         printf("%7.4f seconds, value %9.4f (%.4f)\n", $t1 - $t0, $results[$program], $ratio);
         $worst[$program] = max($worst[$program], $ratio);
      }
   }
   
   // stats
   echo "Stats\n";
   foreach ($programs as $program) {
      echo "   $program", str_pad('', $padding - strlen($program)), ' ';
      printf("%.4f\n", $worst[$program]);
   }
   
   // drawing
   function execute_command($command, $input) {
      $process = proc_open($command, [ 0 => [ 'pipe', 'r' ], 1 => [ 'pipe', 'w' ] ], $pipes);
      fwrite($pipes[0], $input); fclose($pipes[0]); 
      $output = stream_get_contents($pipes[1]);
      fclose($pipes[1]);
      proc_close($process);
      return $output;
   }
   
   foreach ($instances as $instance) {
      if (max(array_map('intval', array_slice(explode('_', $instance), 0, 2))) > 50) {    // do not draw large instances
         continue;
      }
      
      echo "Drawing {$instance}...\n";
      $input = fopen("instances/{$instance}.in", 'r');
      fscanf($input, "%d%d", $a, $b);
      $points = [ ];
      for ($i = 0; $i < $a + $b; ++$i) {
         fscanf($input, "%f%f", $x, $y);
         $points[] = [ $x, $y ];
      }
      fclose($input);

      $table_a = implode("\n", array_map(fn($p) => implode(' ', $p), array_slice($points,  0, $a)));
      $table_b = implode("\n", array_map(fn($p) => implode(' ', $p), array_slice($points, $a, $b)));
      $table_voronoi_a = execute_command('octave -q compute_voronoi.m', "{$a}\n{$table_a}\n");
      $table_voronoi_b = execute_command('octave -q compute_voronoi.m', "{$b}\n{$table_b}\n");
      
      foreach ($programs as $program) {
         echo "   {$program}...\n";
         
         $edges = [ ];
         $input = fopen("logs/{$instance}_{$program}.out", 'r');
         fscanf($input, "%d", $n);
         $edges = [ ];
         for ($i = 0; $i < $n; ++$i) {
            fscanf($input, "%d%d", $p1, $p2);
            $edges[] = [
               [ $points[$p1][0], $points[$p1][1] ],
               [ $points[$p2][0], $points[$p2][1] ]
            ];
         }
         fclose($input);
         $table_edges = implode("\n\n", array_map(fn($a) => implode("\n", array_map(fn($p) => implode(' ', $p), $a)), $edges));
         ob_start( );
?>
\documentclass[pdftex]{article}
\usepackage[utf8]{inputenc}
\usepackage{adjustbox}
\usepackage{caption}
\usepackage[margin=0cm]{geometry}
\usepackage{pgfplots}
\pgfplotsset{compat=1.17}
\begin{document}
   \begin{figure}
   \centering
   \begin{tikzpicture}
      \begin{axis}[
          axis equal, width=18cm, height=18cm
      ]
      \addplot [only marks, blue] table { <?php echo "\n", $table_a, "\n"; ?> };
      \addplot [only marks,  red] table { <?php echo "\n", $table_b, "\n"; ?> };
      \addplot [no markers, update limits=false,  red!25] table { <?php echo "\n", $table_voronoi_b, "\n"; ?> };
      \addplot [no markers, thick] table { <?php echo "\n", $table_edges, "\n"; ?> };
      \end{axis}
   \end{tikzpicture}
   \caption*{<?php echo "Instance ", str_replace('_', '\_', $instance), ' - ', str_replace('_', '\_', $program); ?> }
   \end{figure}
\end{document}
<?php
         $output = ob_get_clean( );
         file_put_contents("tex/{$instance}_{$program}.tex", $output);
         system("pdflatex -output-directory pdf tex/{$instance}_{$program}.tex  > /dev/null 2>&1");
         unlink("pdf/{$instance}_{$program}.aux");
         unlink("pdf/{$instance}_{$program}.log");
      }
   }
?>