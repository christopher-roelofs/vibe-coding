#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <algorithm>

namespace fs = std::filesystem;

// Function to get all image files in a directory
std::vector<std::string> get_image_files(const std::string& directory) {
    std::vector<std::string> image_files;
    for (const auto& entry : fs::directory_iterator(directory)) {
        if (entry.is_regular_file()) {
            std::string ext = entry.path().extension().string();
            if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp") {
                image_files.push_back(entry.path().string());
            }
        }
    }
    // Sort files by modification time, most recent first
    std::sort(image_files.begin(), image_files.end(), 
        [](const std::string& a, const std::string& b) {
            return fs::last_write_time(a) > fs::last_write_time(b);
        });
    return image_files;
}

enum FilterMode
{
    FILTER_NONE = 0,
    FILTER_GRAYSCALE = 1,
    FILTER_SEPIA = 2,
    FILTER_NEGATIVE = 3,
    FILTER_VIGNETTE = 4,
    FILTER_CRT = 5,
    FILTER_PIXELATE = 6,
    FILTER_COLOR_BOOST = 7,
    FILTER_PSYCHEDELIC = 8,
    FILTER_SKETCH = 9,
    FILTER_OILPAINT = 10,
    FILTER_THERMAL = 11,
    FILTER_COOL = 12,
    FILTER_FISHEYE = 13
};

const char *filter_mode_name(FilterMode mode)
{
    switch (mode)
    {
    case FILTER_NONE:
        return "Filter: None";
    case FILTER_GRAYSCALE:
        return "Filter: Grayscale";
    case FILTER_SEPIA:
        return "Filter: Sepia";
    case FILTER_NEGATIVE:
        return "Filter: Negative";
    case FILTER_VIGNETTE:
        return "Filter: Vignette";
    case FILTER_CRT:
        return "Filter: CRT";
    case FILTER_PIXELATE:
        return "Filter: Pixelate";
    case FILTER_COLOR_BOOST:
        return "Filter: Color Boost";
    case FILTER_PSYCHEDELIC:
        return "Filter: Psychedelic";
    case FILTER_SKETCH:
        return "Filter: Sketch";
    case FILTER_OILPAINT:
        return "Filter: Oil Painting";
    case FILTER_THERMAL:
        return "Filter: Thermal";
    case FILTER_COOL:
        return "Filter: Cool Tint";
    case FILTER_FISHEYE:
        return "Filter: Fish-Eye";
    default:
        return "Unknown";
    }
}

void apply_grayscale(cv::Mat &frame)
{
    cv::cvtColor(frame, frame, cv::COLOR_RGB2GRAY);
    cv::cvtColor(frame, frame, cv::COLOR_GRAY2RGB);
}

void apply_sepia(cv::Mat &frame)
{
    for (int y = 0; y < frame.rows; ++y)
    {
        for (int x = 0; x < frame.cols; ++x)
        {
            cv::Vec3b &px = frame.at<cv::Vec3b>(y, x);
            uint8_t r = px[0], g = px[1], b = px[2];
            int tr = std::min(255, static_cast<int>(0.393 * r + 0.769 * g + 0.189 * b));
            int tg = std::min(255, static_cast<int>(0.349 * r + 0.686 * g + 0.168 * b));
            int tb = std::min(255, static_cast<int>(0.272 * r + 0.534 * g + 0.131 * b));
            px = cv::Vec3b(tr, tg, tb);
        }
    }
}

void apply_negative(cv::Mat &frame)
{
    cv::bitwise_not(frame, frame); // Inverts the colors
}

void apply_vignette(cv::Mat &frame)
{
    int width = frame.cols;
    int height = frame.rows;

    // Generate vignette effect based on distance from the center
    cv::Mat mask = cv::Mat::zeros(frame.size(), frame.type());
    cv::Point center(width / 2, height / 2);
    float max_distance = std::sqrt(center.x * center.x + center.y * center.y);

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            float distance = std::sqrt(std::pow(x - center.x, 2) + std::pow(y - center.y, 2));
            float alpha = 1 - (distance / max_distance);
            alpha = std::max(0.3f, alpha); // Minimum alpha value

            cv::Vec3b &px = frame.at<cv::Vec3b>(y, x);
            px[0] = static_cast<uint8_t>(px[0] * alpha);
            px[1] = static_cast<uint8_t>(px[1] * alpha);
            px[2] = static_cast<uint8_t>(px[2] * alpha);
        }
    }
}

// CRT filter: applies scanlines and color bleed
void apply_crt(cv::Mat &frame)
{
    int width = frame.cols;
    int height = frame.rows;

    // Add scanline effect
    for (int y = 0; y < height; ++y)
    {
        if (y % 2 == 0)
        { // Apply scanlines every other row
            for (int x = 0; x < width; ++x)
            {
                cv::Vec3b &px = frame.at<cv::Vec3b>(y, x);
                px[0] = cv::saturate_cast<uchar>(px[0] * 0.8); // Red channel darkened
                px[1] = cv::saturate_cast<uchar>(px[1] * 0.8); // Green channel darkened
                px[2] = cv::saturate_cast<uchar>(px[2] * 0.8); // Blue channel darkened
            }
        }
    }

    // Simulate color bleed by slightly offsetting each color channel
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            cv::Vec3b &px = frame.at<cv::Vec3b>(y, x);

            // Slightly shift the color channels
            px[0] = cv::saturate_cast<uchar>(px[0] + (rand() % 10 - 5)); // Red
            px[1] = cv::saturate_cast<uchar>(px[1] + (rand() % 10 - 5)); // Green
            px[2] = cv::saturate_cast<uchar>(px[2] + (rand() % 10 - 5)); // Blue
        }
    }

    // Apply slight curvature distortion to simulate the CRT curve
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            cv::Vec3b &px = frame.at<cv::Vec3b>(y, x);

            // Calculate the distance from the center of the image
            double dist = std::sqrt(std::pow(x - width / 2, 2) + std::pow(y - height / 2, 2));
            double factor = 1.0 - (dist / (std::sqrt(std::pow(width / 2, 2) + std::pow(height / 2, 2))));

            // Apply curvature effect by slightly distorting the pixel color based on distance from center
            px[0] = cv::saturate_cast<uchar>(px[0] * factor);
            px[1] = cv::saturate_cast<uchar>(px[1] * factor);
            px[2] = cv::saturate_cast<uchar>(px[2] * factor);
        }
    }
}

void apply_pixelate(cv::Mat &frame, int pixel_size = 10)
{
    int width = frame.cols;
    int height = frame.rows;

    for (int y = 0; y < height; y += pixel_size)
    {
        for (int x = 0; x < width; x += pixel_size)
        {
            // Get the block's region of interest (ROI)
            cv::Rect block(x, y, pixel_size, pixel_size);
            cv::Mat blockROI = frame(block);

            // Calculate the average color in the block
            cv::Scalar avg_color = cv::mean(blockROI);

            // Fill the block with the average color
            for (int i = 0; i < pixel_size && y + i < height; ++i)
            {
                for (int j = 0; j < pixel_size && x + j < width; ++j)
                {
                    frame.at<cv::Vec3b>(y + i, x + j) = cv::Vec3b(avg_color[0], avg_color[1], avg_color[2]);
                }
            }
        }
    }
}

void apply_color_boost(cv::Mat &frame)
{
    cv::Mat hsv;
    cv::cvtColor(frame, hsv, cv::COLOR_RGB2HSV);
    for (int y = 0; y < hsv.rows; ++y)
    {
        for (int x = 0; x < hsv.cols; ++x)
        {
            cv::Vec3b &px = hsv.at<cv::Vec3b>(y, x);
            px[1] = std::min(255, static_cast<int>(px[1] * 1.8)); // Increase saturation
            px[2] = std::min(255, static_cast<int>(px[2] * 1.3)); // Increase brightness
        }
    }
    cv::cvtColor(hsv, frame, cv::COLOR_HSV2RGB);
}

void apply_psychedelic(cv::Mat &frame)
{
    cv::Mat hsv;
    cv::cvtColor(frame, hsv, cv::COLOR_RGB2HSV);

    auto time_now = std::chrono::system_clock::now();
    float time_factor = std::chrono::duration_cast<std::chrono::milliseconds>(
                            time_now.time_since_epoch())
                            .count() /
                        300.0f;

    for (int y = 0; y < hsv.rows; ++y)
    {
        for (int x = 0; x < hsv.cols; ++x)
        {
            cv::Vec3b &px = hsv.at<cv::Vec3b>(y, x);
            int hue_shift = static_cast<int>(90 * std::sin((x + y + time_factor) * 0.01));
            px[0] = (px[0] + hue_shift + 180) % 180;
        }
    }

    cv::cvtColor(hsv, frame, cv::COLOR_HSV2RGB);
}

void apply_cool_tint(cv::Mat &frame)
{
    cv::Mat cool = frame.clone();

    // Reduce red channel slightly and boost blue
    for (int y = 0; y < cool.rows; ++y)
    {
        for (int x = 0; x < cool.cols; ++x)
        {
            cv::Vec3b &px = cool.at<cv::Vec3b>(y, x);
            // BGR order
            px[0] = cv::saturate_cast<uchar>(px[0] + 30); // Boost Blue
            px[1] = cv::saturate_cast<uchar>(px[1] + 10); // Slightly boost Green
            px[2] = cv::saturate_cast<uchar>(px[2] - 20); // Reduce Red
        }
    }

    frame = cool;
}

void apply_fisheye(cv::Mat &frame)
{
    int width = frame.cols;
    int height = frame.rows;
    cv::Mat distorted = cv::Mat::zeros(frame.size(), frame.type());

    float k = 0.00001f; // Strength of distortion
    float centerX = width / 2.0f;
    float centerY = height / 2.0f;

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            float dx = (x - centerX);
            float dy = (y - centerY);
            float r = sqrt(dx * dx + dy * dy);
            float factor = 1.0f + k * r * r;
            int srcX = static_cast<int>(centerX + dx / factor);
            int srcY = static_cast<int>(centerY + dy / factor);

            if (srcX >= 0 && srcX < width && srcY >= 0 && srcY < height)
            {
                distorted.at<cv::Vec3b>(y, x) = frame.at<cv::Vec3b>(srcY, srcX);
            }
        }
    }

    frame = distorted;
}

void apply_sketch(cv::Mat &frame)
{
    cv::Mat gray, blurImg, edges;
    cv::cvtColor(frame, gray, cv::COLOR_RGB2GRAY);
    cv::GaussianBlur(gray, blurImg, cv::Size(5, 5), 0);
    cv::Laplacian(blurImg, edges, CV_8U, 5);
    cv::bitwise_not(edges, edges);
    cv::cvtColor(edges, frame, cv::COLOR_GRAY2RGB);
}

void apply_oil_painting(cv::Mat &frame)
{
    cv::Mat result;
    cv::stylization(frame, result, 60, 0.45); // Stylization with custom parameters
    frame = result;
}

void apply_thermal(cv::Mat &frame)
{
    cv::Mat gray, colored;
    cv::cvtColor(frame, gray, cv::COLOR_RGB2GRAY);
    cv::applyColorMap(gray, colored, cv::COLORMAP_JET);
    cv::cvtColor(colored, frame, cv::COLOR_BGR2RGB); // Convert to RGB for SDL
}

std::string timestamped_filename(const std::string &folder)
{
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << folder << "/snapshot_" << std::put_time(std::localtime(&t), "%Y%m%d_%H%M%S") << ".png";
    return ss.str();
}

void renderText(SDL_Renderer *renderer, TTF_Font *font, const std::string &text, int x, int y)
{
    SDL_Color white = {255, 255, 255};
    SDL_Color black = {0, 0, 0};

    // Render the black outline by drawing the text in 8 surrounding directions
    for (int dx = -1; dx <= 1; ++dx)
    {
        for (int dy = -1; dy <= 1; ++dy)
        {
            if (dx == 0 && dy == 0)
                continue; // Skip center
            SDL_Surface *shadow = TTF_RenderText_Blended(font, text.c_str(), black);
            SDL_Texture *shadowTex = SDL_CreateTextureFromSurface(renderer, shadow);
            SDL_Rect shadowRect = {x + dx, y + dy, shadow->w, shadow->h};
            SDL_RenderCopy(renderer, shadowTex, nullptr, &shadowRect);
            SDL_FreeSurface(shadow);
            SDL_DestroyTexture(shadowTex);
        }
    }

    // Render the main white text
    SDL_Surface *surface = TTF_RenderText_Blended(font, text.c_str(), white);
    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect dst = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, textTexture, nullptr, &dst);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(textTexture);
}

int main(int argc, char *argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0 || TTF_Init() != 0 || Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        std::cerr << "SDL/TTF/Mixer Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Setup capture and folder
    std::string image_folder = "images";
    fs::create_directories(image_folder);

    cv::VideoCapture cap(0);
    if (!cap.isOpened())
    {
        std::cerr << "Failed to open webcam" << std::endl;
        SDL_Quit();
        return 1;
    }

    int camWidth = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    int camHeight = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));

    SDL_Window *window = SDL_CreateWindow("Webcam Viewer",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          camWidth, camHeight,
                                          SDL_WINDOW_FULLSCREEN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture *texture = SDL_CreateTexture(renderer,
                                             SDL_PIXELFORMAT_RGB24,
                                             SDL_TEXTUREACCESS_STREAMING,
                                             camWidth, camHeight);

    TTF_Font *font = TTF_OpenFont("DejaVuSans.ttf", 24);
    if (!font)
    {
        std::cerr << "Failed to load font" << std::endl;
        SDL_Quit();
        return 1;
    }

    Mix_Chunk *shutterSound = Mix_LoadWAV("shutter.wav");
    if (!shutterSound)
    {
        std::cerr << "Failed to load shutter.wav" << std::endl;
    }

    bool running = true;
    FilterMode currentFilter = FILTER_NONE;
    bool galleryMode = false;
    std::vector<std::string> galleryImages;
    size_t currentImageIndex = 0;
    bool deletionConfirmationMode = false;
    
    // Capture notification variables
    std::string captureNotification;
    Uint32 captureNotificationStart = 0;
    const Uint32 NOTIFICATION_DURATION = 3000; // 3 seconds
    SDL_Event event;

    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                running = false;
            else if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {
                case SDLK_ESCAPE:
                    if (galleryMode) {
                        // Exit gallery mode
                        galleryMode = false;
                    } else {
                        running = false;
                    }
                    break;
                case SDLK_RIGHT:
                    if (galleryMode) {
                        // Next image in gallery
                        if (!galleryImages.empty()) {
                            currentImageIndex = (currentImageIndex + 1) % galleryImages.size();
                        }
                    } else {
                        // Cycle to next filter
                        currentFilter = static_cast<FilterMode>((static_cast<int>(currentFilter) + 1) % 14);
                    }
                    break;
                case SDLK_LEFT:
                    if (galleryMode) {
                        // Previous image in gallery
                        if (!galleryImages.empty()) {
                            currentImageIndex = (currentImageIndex - 1 + galleryImages.size()) % galleryImages.size();
                        }
                    } else {
                        // Cycle to previous filter
                        currentFilter = static_cast<FilterMode>((static_cast<int>(currentFilter) - 1 + 14) % 14);
                    }
                    break;
                case SDLK_g:
                    // Toggle gallery mode
                    galleryMode = !galleryMode;
                    if (galleryMode) {
                        // Populate gallery images
                        galleryImages = get_image_files(image_folder);
                        currentImageIndex = 0;
                    }
                    break;
                case SDLK_d:
                    // Delete image in gallery mode
                    if (galleryMode && !galleryImages.empty()) {
                        // Enter deletion confirmation mode
                        deletionConfirmationMode = true;
                    }
                    break;
                case SDLK_a:
                    // Confirm deletion in gallery mode
                    if (galleryMode && deletionConfirmationMode) {
                        try {
                            fs::remove(galleryImages[currentImageIndex]);
                            std::cout << "Deleted: " << galleryImages[currentImageIndex] << std::endl;
                            
                            // Remove from gallery list
                            galleryImages.erase(galleryImages.begin() + currentImageIndex);
                            
                            // Adjust index if needed
                            if (galleryImages.empty()) {
                                galleryMode = false;
                            } else {
                                currentImageIndex %= galleryImages.size();
                            }
                            
                            // Reset deletion confirmation
                            deletionConfirmationMode = false;
                        } catch (const fs::filesystem_error& e) {
                            std::cerr << "Error deleting file: " << e.what() << std::endl;
                            deletionConfirmationMode = false;
                        }
                    }
                    break;
                case SDLK_c:
                    // Cancel deletion in gallery mode
                    if (galleryMode && deletionConfirmationMode) {
                        deletionConfirmationMode = false;
                    }
                    break;
                case SDLK_s:
                {
                    std::string path = timestamped_filename(image_folder);
                    cv::Mat snap;
                    cap.read(snap);
                    if (currentFilter == FILTER_GRAYSCALE)
                        apply_grayscale(snap);
                    else if (currentFilter == FILTER_SEPIA)
                        apply_sepia(snap);
                    else if (currentFilter == FILTER_NEGATIVE)
                        apply_negative(snap);
                    else if (currentFilter == FILTER_VIGNETTE)
                        apply_vignette(snap);
                    else if (currentFilter == FILTER_CRT)
                        apply_crt(snap);
                    else if (currentFilter == FILTER_PIXELATE)
                        apply_pixelate(snap, 10); // Default pixel size of 10
                    else if (currentFilter == FILTER_COLOR_BOOST)
                        apply_color_boost(snap);
                    else if (currentFilter == FILTER_PSYCHEDELIC)
                        apply_psychedelic(snap);
                    else if (currentFilter == FILTER_SKETCH)
                        apply_sketch(snap);
                    else if (currentFilter == FILTER_OILPAINT)
                        apply_oil_painting(snap);
                    else if (currentFilter == FILTER_THERMAL)
                        apply_thermal(snap);
                    else if (currentFilter == FILTER_COOL)
                        apply_cool_tint(snap);
                    else if (currentFilter == FILTER_FISHEYE)
                        apply_fisheye(snap);
                    cv::imwrite(path, snap);
                    if (shutterSound)
                        Mix_PlayChannel(-1, shutterSound, 0);
                    std::cout << "Saved image to " << path << std::endl;
                    
                    // Set capture notification
                    captureNotification = "Image Saved: " + fs::path(path).filename().string();
                    captureNotificationStart = SDL_GetTicks();
                    break;
                }
                }
            }
        }

        cv::Mat frame;
        if (galleryMode) {
            // Gallery mode: display saved images
            if (!galleryImages.empty()) {
                frame = cv::imread(galleryImages[currentImageIndex]);
                if (frame.empty()) {
                    // Remove invalid image from gallery
                    galleryImages.erase(galleryImages.begin() + currentImageIndex);
                    if (galleryImages.empty()) {
                        galleryMode = false;
                        continue;
                    }
                    currentImageIndex %= galleryImages.size();
                    frame = cv::imread(galleryImages[currentImageIndex]);
                }
                cv::resize(frame, frame, cv::Size(camWidth, camHeight));
                cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
            } else {
                // No images in gallery
                galleryMode = false;
                continue;
            }
        } else {
            // Normal camera mode
            cap >> frame;
            if (frame.empty())
                continue;

            cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);

            if (currentFilter == FILTER_GRAYSCALE)
                apply_grayscale(frame);
            else if (currentFilter == FILTER_SEPIA)
                apply_sepia(frame);
            else if (currentFilter == FILTER_NEGATIVE)
                apply_negative(frame);
            else if (currentFilter == FILTER_VIGNETTE)
                apply_vignette(frame);
            else if (currentFilter == FILTER_CRT)
                apply_crt(frame);
            else if (currentFilter == FILTER_PIXELATE)
                apply_pixelate(frame, 10); // Default pixel size of 10
            else if (currentFilter == FILTER_COLOR_BOOST)
                apply_color_boost(frame);
            else if (currentFilter == FILTER_PSYCHEDELIC)
                apply_psychedelic(frame);
            else if (currentFilter == FILTER_SKETCH)
                apply_sketch(frame);
            else if (currentFilter == FILTER_OILPAINT)
                apply_oil_painting(frame);
            else if (currentFilter == FILTER_THERMAL)
                apply_thermal(frame);
            else if (currentFilter == FILTER_COOL)
                apply_cool_tint(frame);
            else if (currentFilter == FILTER_FISHEYE)
                apply_fisheye(frame);
        }

        SDL_UpdateTexture(texture, nullptr, frame.data, frame.step);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);

        // Draw overlay
        std::string overlayText;
        if (galleryMode) {
            if (!galleryImages.empty()) {
                overlayText = "Gallery: " + std::to_string(currentImageIndex + 1) + "/" + std::to_string(galleryImages.size());
            } else {
                overlayText = "No images in gallery";
            }
        } else {
            overlayText = filter_mode_name(currentFilter);
        }
        renderText(renderer, font, overlayText, 20, camHeight - 40);

        // Draw deletion confirmation text
        if (deletionConfirmationMode) {
            std::string confirmText = "Press A to CONFIRM or C to CANCEL";
            renderText(renderer, font, confirmText, 
                      camWidth / 2 - (confirmText.length() * 5), 
                      camHeight - 30);
        }

        // Draw capture notification if active
        if (!captureNotification.empty() && SDL_GetTicks() - captureNotificationStart < NOTIFICATION_DURATION) {
            renderText(renderer, font, captureNotification, 20, camHeight - 10);
        }

        SDL_RenderPresent(renderer);
    }

    Mix_FreeChunk(shutterSound);
    TTF_CloseFont(font);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_CloseAudio();
    TTF_Quit();
    SDL_Quit();
    return 0;
}
