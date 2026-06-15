#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Eng_TechnicalArchitect.generated.h"

UENUM(BlueprintType)
enum class EEng_ArchitecturalLayer : uint8
{
    Core = 0,           // Engine, Physics, Memory
    World = 1,          // Terrain, Biomes, Weather
    Character = 2,      // Player, NPCs, Movement
    AI = 3,             // Behavior Trees, Crowd Simulation
    Audio = 4,          // Sound, Music, Voice
    VFX = 5,            // Particles, Post-Process
    UI = 6              // HUD, Menus, Inventory
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ModuleInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly)
    EEng_ArchitecturalLayer Layer;

    UPROPERTY(BlueprintReadOnly)
    int32 Priority;

    UPROPERTY(BlueprintReadOnly)
    TArray<FString> Dependencies;

    UPROPERTY(BlueprintReadOnly)
    bool bIsInitialized;

    UPROPERTY(BlueprintReadOnly)
    float InitializationTime;

    FEng_ModuleInfo()
    {
        ModuleName = TEXT("");
        Layer = EEng_ArchitecturalLayer::Core;
        Priority = 0;
        bIsInitialized = false;
        InitializationTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceTargets
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    float TargetFPS;

    UPROPERTY(BlueprintReadOnly)
    int32 MaxActors;

    UPROPERTY(BlueprintReadOnly)
    float MaxMemoryMB;

    UPROPERTY(BlueprintReadOnly)
    float MaxDrawCalls;

    FEng_PerformanceTargets()
    {
        TargetFPS = 60.0f;
        MaxActors = 8000;
        MaxMemoryMB = 4096.0f;
        MaxDrawCalls = 2000.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_TechnicalArchitect : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_TechnicalArchitect();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    void RegisterModule(const FString& ModuleName, EEng_ArchitecturalLayer Layer, int32 Priority, const TArray<FString>& Dependencies);

    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    bool ValidateModuleDependencies();

    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    void InitializeModulesInOrder();

    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    bool IsModuleInitialized(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    FEng_PerformanceTargets GetPerformanceTargets() const;

    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    bool ValidatePerformance();

    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    TArray<FString> GetCircularDependencies();

    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    void SetPerformanceTarget(float FPS, int32 MaxActors, float MaxMemoryMB);

    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    TArray<FEng_ModuleInfo> GetModulesByLayer(EEng_ArchitecturalLayer Layer);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Technical Architecture")
    TArray<FEng_ModuleInfo> RegisteredModules;

    UPROPERTY(BlueprintReadOnly, Category = "Technical Architecture")
    FEng_PerformanceTargets PerformanceTargets;

    UPROPERTY(BlueprintReadOnly, Category = "Technical Architecture")
    bool bArchitectureValidated;

    UPROPERTY(BlueprintReadOnly, Category = "Technical Architecture")
    float LastValidationTime;

private:
    void SortModulesByPriority();
    bool HasCircularDependency(const FString& ModuleName, TArray<FString>& VisitedModules);
    FEng_ModuleInfo* FindModule(const FString& ModuleName);
    void InitializeModule(FEng_ModuleInfo& Module);
    bool CheckDependenciesInitialized(const FEng_ModuleInfo& Module);
};