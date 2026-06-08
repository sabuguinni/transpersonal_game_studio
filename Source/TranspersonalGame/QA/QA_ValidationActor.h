#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QA_TestFramework.h"
#include "QA_ValidationActor.generated.h"

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQA_ValidationActor : public AActor
{
    GENERATED_BODY()
    
public:    
    AQA_ValidationActor();

protected:
    virtual void BeginPlay() override;

public:    
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    void StartValidationSuite();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    void ValidateCurrentLevel();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    void ValidatePlayerCharacter();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    void ValidateDinosaurActors();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    void ValidateWorldAssets();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    void ValidatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "QA Validation")
    void RunEditorValidation();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Components")
    class UStaticMeshComponent* ValidationMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    bool bAutoValidateOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    float ValidationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    bool bShowValidationResults;

    UPROPERTY(BlueprintReadOnly, Category = "QA Results")
    TArray<FQA_TestCase> LastValidationResults;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Framework")
    UQA_TestFramework* TestFramework;

private:
    float LastValidationTime;
    bool bValidationInProgress;

    void InitializeTestFramework();
    void DisplayValidationResults();
    FString GetValidationSummary() const;
};