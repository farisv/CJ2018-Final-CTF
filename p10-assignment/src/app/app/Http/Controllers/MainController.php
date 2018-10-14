<?php

namespace App\Http\Controllers;

use Illuminate\Http\Request;
use App\User;
use App\Http\Controllers\Controller;

class MainController extends Controller
{
    public function show(Request $request)
    {
        $dir_name = '/tmp/' . $request->session()->getId();
        if (file_exists($dir_name)) {
            $files = glob($dir_name . '/*');
            $checked = array("c", "cpp", "py", "sh");
            $result = array();
            foreach ($files as $file) {
                if (is_file($file)) {
                    $exp = explode('.', $file);
                    $extension = end($exp);
                    $exp = explode('/', $file);
                    $filename = end($exp);
                    if (in_array($extension, $checked)) {
                        $content = file_get_contents($file);
                        $result[$filename] = sha1($content);                        
                    }
                }
            }
            return view('main', ['result' => $result]);
        }
        return view('main');
    }

    public function upload(Request $request)
    {
        if ($request->hasFile('file')) {
            $dir_name = '/tmp/' . $request->session()->getId();
            if (!file_exists($dir_name)) {
                mkdir($dir_name);
            } else {
                $files = glob($dir_name . '/*');
                foreach ($files as $file) {
                    if (is_file($file)) {
                        unlink($file);
                    }
                }
            }

            $file = request()->file('file');
            $extension = $file->getClientOriginalExtension();

            $allowed = array("tar", "zip", "phar");
            if (in_array($extension, $allowed)) {
                $name = $file->getClientOriginalName();
                $file->move($dir_name . '/', $name);
                $path = $dir_name . '/' . $name;

                $archive = NULL;

                try {
                    $archive = new \Phar($path);
                } catch (\Exception $e) {
                    try {
                        $archive = new \PharData($path);
                    } catch (\Exception $e) {
                        // pass
                    }
                }

                if ($archive) {
                    $archive->extractTo($dir_name);
                    $checked = array("c", "cpp", "py", "sh");
                    $result = array();
                    foreach (new \RecursiveIteratorIterator($archive) as $submission) {
                        $exp = explode('.', $submission);
                        $extension = end($exp);
                        $exp = explode('/', $submission);
                        $filename = end($exp);
                        if (in_array($extension, $checked)) {
                            $content = file_get_contents($submission);
                            $result[$filename] = sha1($content);
                        }
                    }
                    return view('main', ['result' => $result]);
                } else {
                    return view('main', ['error' => "Invalid Archive"]);
                }

            } else {
                return view('main', ['error' => "Invalid Archive"]);
            }
        }
        return view('main');
    }
}
