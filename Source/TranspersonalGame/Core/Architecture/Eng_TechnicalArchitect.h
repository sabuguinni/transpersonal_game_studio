#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Eng_TechnicalArchitect.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ArchitecturalLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FString LayerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    int32 Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bIsInitialized;

    FEng_ArchitecturalLayer()
    {
        LayerName = TEXT("");
        Priority = 0;
        bIsInitialized = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ModuleDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    FString ModuleName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    FString LayerAssignment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    TArray<FString> RequiredModules;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    bool bIsCompiled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    float PerformanceWeight;

    FEng_ModuleDefinition()
    {
        ModuleName = TEXT("");
        LayerAssignment = TEXT("");
        bIsCompiled = false;
        PerformanceWeight = 1.0f;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UEng_TechnicalArchitect : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core Architecture Management
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void DefineArchitecturalLayers();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool RegisterModule(const FString& ModuleName, const FString& Layer, const TArray<FString>& Dependencies);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateArchitecture();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FString> GetInitializationOrder();

    // Performance Architecture
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceTargets(float TargetFPS, int32 MaxActors, float MaxMemoryMB);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool CheckPerformanceCompliance();

    // Module Communication Standards
    UFUNCTION(BlueprintCallable, Category = "Communication")
    void DefineInterfaceStandards();

    UFUNCTION(BlueprintCallable, Category = "Communication")
    bool ValidateModuleInterfaces();

    // Technical Standards Enforcement
    UFUNCTION(BlueprintCallable, Category = "Standards")
    void EnforceCodingStandards();

    UFUNCTION(BlueprintCallable, Category = "Standards")
    TArray<FString> GetArchitectureViolations();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture")
    TArray<FEng_ArchitecturalLayer> ArchitecturalLayers;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture")
    TArray<FEng_ModuleDefinition> RegisteredModules;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxMemoryUsage;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Validation")
    TArray<FString> ArchitectureViolations;

private:
    void InitializeCoreArchitecture();
    void ValidateModuleDependencies();
    bool CheckCircularDependencies();
    void GenerateInitializationSequence();
};