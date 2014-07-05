#ifndef ETAPP_H
#define ETAPP_H

class ETModel;
class GLWidget;

class ETApp
{
public:
    /**
     * @brief All resources for this project are found here.
     */
    ETApp();

    /**
     * @brief Acces to the model.
     * This is the 3D mode tht we try to edit. In a later version, we may add
     * support for multiple models ina scene.
     * @return a pointer to the model.
     */
    ETModel *model() { return pModel; }

    /**
     * @brief Create a new model and return a pointer to it.
     * As we have currently support only a single model, this will delete
     * the current model and create a new, empty model.
     * @return
     */
    ETModel *newModel();

    /**
     * @brief Load a model from local storage.
     * @param filename
     * @return a pointer to the new model
     */
    ETModel *loadModel(const char *filename);

    /**
     * @brief Trigger a redraw of the scene.
     * This is usually called when the model data cahnged and will cause
     * a newly rendered scene as soon as the code returns to the main loop.
     */
    void redrawScene();

    /**
     * @brief This is needed for telling the renderer that the model changed.
     */
    void linkWith(GLWidget *w) { pGLWidget = w; }

private:
    ETModel *pModel;
    GLWidget *pGLWidget;
};


extern ETApp ET;

#endif // ETAPP_H
