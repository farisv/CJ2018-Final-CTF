<?php

abstract class Image_Driver_Base extends EmptyIterator
{
    /**
     * Background color.
     *
     * @var string
     */
    protected $_background = 'white';
    /**
     * Capabilites of this driver.
     *
     * @var array
     */
    protected $_capabilities = array();
    /**
     * The current image data.
     *
     * @var string
     */
    protected $_data = '';
    /**
     * Logger.
     */
    protected $_logger;
    /**
     * The current width of the image data.
     *
     * @var integer
     */
    protected $_width = 0;
    /**
     * The current height of the image data.
     *
     * @var integer
     */
    protected $_height = 0;
    /**
     * A directory for temporary files.
     *
     * @var string
     */
    protected $_tmpdir;
    /**
     * Array containing available Effects
     *
     * @var array
     */
    protected $_loadedEffects = array();
    /**
     * What kind of images should ImageMagick generate? Defaults to 'png'.
     *
     * @var string
     */
    protected $_type = 'png';
    /**
     * Cache the context
     *
     * @param array
     */
     protected $_context;
    /**
     * Constructor.
     *
     * @param array $params   The image object parameters. Values include:
     *                        - background: (string) The background color.
     *                                      DEFAULT: white.
     *                        - data: (string) The image binary data.
     *                        - height: (integer) The desired image height.
     *                        - type: (string) The output image type (png, jpeg
     *                                etc.). DEFAULT: png.
     *                        - width: (integer) The desired image width.
     * @param array $context  The object context - configuration, injected
     *                        objects:
     *                        - logger: A logger.
     *                        - tmpdir: [REQUIRED] (string) Temporary directory.
     *
     * @throws InvalidArgumentException
     */
    protected function __construct($params, $context = array())
    {
        $this->_params = $params;
        $this->_context = $context;
        if (empty($context['tmpdir'])) {
            throw new InvalidArgumentException(
                'A path to a temporary directory is required.'
            );
        }
        $this->_tmpdir = $context['tmpdir'];
        if (!empty($context['logger'])) {
            $this->_logger = $context['logger'];
        }
        if (isset($params['width'])) {
            $this->_width = $params['width'];
        }
        if (isset($params['height'])) {
            $this->_height = $params['height'];
        }
        if (!empty($params['type'])) {
            // We only want the extension, not the full mimetype.
            if (strpos($params['type'], 'image/') !== false) {
                $params['type'] = substr($params['type'], 6);
            }
            $this->_type = $params['type'];
        }
        if (!empty($params['background'])) {
            $this->_background = $params['background'];
        }
    }
    /**
     * Catch-all method so that we don't error out when calling an unsupported
     * manipulation method.
     */
    public function __call($method, $args)
    {
    }
    /**
     * Returns the capabilities.
     *
     * @return array  A list of backend capabilities.
     */
    public function getCapabilities()
    {
        return $this->_capabilities;
    }
    /**
     * Checks the existence of a particular capability.
     *
     * @param string $capability  The capability to check for.
     *
     * @return boolean  True if the backend has this capability.
     */
    public function hasCapability($capability)
    {
        return in_array($capability, $this->_capabilities);
    }
    /**
     * Sends HTTP headers for the image.
     */
    public function headers()
    {
        header('Content-type: ' . $this->getContentType());
    }
    /**
     * Returns the MIME type for this image.
     *
     * @return string  The MIME type for this image.
     */
    public function getContentType()
    {
        return 'image/' . $this->_type;
    }
    /**
     * Returns the image type.
     *
     * @return string  The type of this image (png, jpg, etc.).
     */
    public function getType()
    {
        return $this->_type;
    }
    /**
     * Sets the output image type.
     *
     * @param string $type  An image type (png, jpg, etc.)
     *
     * @return string  The previous image type.
     */
    public function setType($type)
    {
        // We only want the extension, not the full mimetype.
        if (strpos($type, 'image/') !== false) {
            $type = substr($type, 6);
        }
        $old = $this->_type;
        $this->_type = $type;
        return $old;
    }
    /**
     * Draws a shaped point at the specified (x,y) point.
     *
     * Useful for scatter diagrams, debug points, etc. Draws squares, circles,
     * diamonds, and triangles.
     *
     * @param integer $x     The x coordinate of the point to brush.
     * @param integer $y     The y coordinate of the point to brush.
     * @param string $color  The color to brush the point with.
     * @param string $shape  What brush to use? Defaults to a square.
     */
    public function brush($x, $y, $color = 'black', $shape = 'square')
    {
        switch ($shape) {
        case 'triangle':
            $verts[0] = array('x' => $x + 3, 'y' => $y + 3);
            $verts[1] = array('x' => $x, 'y' => $y - 3);
            $verts[2] = array('x' => $x - 3, 'y' => $y + 3);
            $this->polygon($verts, $color, $color);
            break;
        case 'circle':
            $this->circle($x, $y, 3, $color, $color);
            break;
        case 'diamond':
            $verts[0] = array('x' => $x - 3, 'y' => $y);
            $verts[1] = array('x' => $x, 'y' => $y + 3);
            $verts[2] = array('x' => $x + 3, 'y' => $y);
            $verts[3] = array('x' => $x, 'y' => $y - 3);
            $this->polygon($verts, $color, $color);
            break;
        case 'square':
        default:
            $this->rectangle($x - 2, $y - 2, 4, 4, $color, $color);
            break;
        }
    }
    /**
     * Resets the image data to defaults.
     */
    public function reset()
    {
        $this->_data = '';
        $this->_width = null;
        $this->_height = null;
        $this->_background = 'white';
    }
    /**
     * Returns the height and width of the current image data.
     *
     * @return array  An hash with 'width' containing the width,
     *                'height' containing the height of the image.
     */
    public function getDimensions()
    {
        // Check if we know it already
        if ($this->_width == 0 && $this->_height == 0) {
            $tmp = $this->toFile();
            $details = @getimagesize($tmp);
            list($this->_width, $this->_height) = $details;
            unlink($tmp);
        }
        return array('width' => $this->_width, 'height' => $this->_height);
    }
    /**
     * Loads the image data from a string.
     *
     * @param string $image_data  The data to use for the image.
     */
    public function loadString($image_data)
    {
        $this->reset();
        $this->_data = $image_data;
    }
    /**
     * Loads the image data from a file.
     *
     * @param string $filename  The full path and filename to the file to load
     *                          the image data from.
     *
     */
    public function loadFile($filename)
    {
        $this->reset();
        if (!file_exists($filename)) {
            return;
        }
        if (!$this->_data = file_get_contents($filename)) {
            return;
        }
    }
    /**
     * Saves image data to file.
     *
     * If $data is false, saves current image data after performing any pending
     * operations on the data.  If $data contains raw image data, saves that
     * data to file without regard for the current image data.
     *
     * @param string  String of binary image data.
     *
     * @return string  Path to temporary file.
     */
    public function toFile($data = null)
    {
        $tmp = stream_get_meta_data(tmpfile())['uri'];
        $fp = fopen($tmp, 'wb');
        fwrite($fp, $data ?: $this->raw());
        fclose($fp);
        return $tmp;
    }
    /**
     * Displays the current image.
     */
    public function display()
    {
        $this->headers();
        echo $this->raw(true);
    }
    /**
     * Returns the raw data for this image.
     *
     * @param boolean $convert  If true, the image data will be returned in the
     *                          target format, independently from any image
     *                          operations.
     *
     * @return string  The raw image data.
     */
    public function raw($convert = false)
    {
        return $this->_data;
    }
    /**
     * Applies any effects in the effect queue.
     */
    public function applyEffects()
    {
        $this->raw();
    }
    /**
     * Returns the current temporary directory.
     *
     * @return string  The current temporary directory.
     */
    public function getTmpDir()
    {
        return $this->_tmpdir;
    }
    /**
     * Utility function to zero out cached geometry information.
     *
     * Shouldn't really be called from client code, but is needed since effects
     * may need to clear these.
     */
    public function clearGeometry()
    {
        $this->_height = 0;
        $this->_width = 0;
    }
    /**
     * Returns a specific image from the pages of images.
     *
     * @param integer $index  The index to return.
     *
     * @return Image_Driver_Base  The requested image
     */
    public function getImageAtIndex($index)
    {
        $class = get_class($this);
        return new $class(array('data' => $this->raw()), $this->_context);
    }
    /**
     * Returns the number of image pages available in the image object.
     *
     * @return integer  The number of images.
     */
    public function getImagePageCount()
    {
        return 1;
    }
}
