        <!-- jQuery 1.11.3 -->
        <script src="https://ajax.googleapis.com/ajax/libs/jquery/1.11.3/jquery.min.js"></script>

        <!-- Bootstrap JavaScript -->
        <script src="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.5/js/bootstrap.min.js"></script>

        <!-- Charts.js -->
        <script src="js/Chart.js"></script>

        <!-- Meridian 59 -->
        <script src="js/site.js"></script>
        <?php
            $controller_name = basename($_SERVER["SCRIPT_FILENAME"], '.php');
            echo '<script src="js/'.$controller_name.'_controller.js"></script>';
        ?>

    </body>
</html>