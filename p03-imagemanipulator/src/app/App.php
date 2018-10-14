<?php

class App
{
    private function render($template)
    {
        print file_get_contents(TEMPLATE_DIR . $template);
    }

    private function display($type, $data)
    {
        header('Content-type: ' . $type);
        print $data;
    }

    private function manipulate($uploaded, $ops)
    {
        $params = array(
            'type' => $uploaded['type'],
            'filename' => $uploaded['tmp_name']
        );
        $context = array(
            'tmpdir' => sys_get_temp_dir(),
            'convert' => CONVERT_PATH
        );

        $manipulator = new Image_Driver_Convert($params, $context);

        if (isset($ops['resize'])) {
            $manipulator->resize($ops['width'], $ops['height']);
        }

        if (isset($ops['crop'])) {
            $manipulator->crop($ops['x1'], $ops['y1'], $ops['x2'], $opx['y2']);
        }

        if (isset($ops['rotate'])) {
            $manipulator->rotate($ops['angle']);
        }

        if (isset($ops['flip'])) {
            $manipulator->flip();
        }

        if (isset($ops['mirror'])) {
            $manipulator->mirror();
        }

        if (isset($ops['grayscale'])) {
            $manipulator->grayscale();
        }

        if (isset($ops['sepia'])) {
            if (isset($ops['threshold'])) {
                $manipulator->sepia($ops['threshold']);
            } else {
                $manipulator->sepia();
            }
        }

        $this->display($uploaded['type'], $manipulator->raw(true));
    }

    public function run()
    {
        if (isset($_FILES['file'])) {
            $this->manipulate($_FILES['file'], $_POST);
        } else {
            $this->render('main.html');
        }
    }
}
