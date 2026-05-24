#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "BuildValidationManager.generated.h"

UENUM(BlueprintType)
enum class EBuild_ValidationResult : uint8
{
    Success         UMETA(DisplayName = "Success"),
    Warning         UMETA(DisplayName = "Warning"),
    Error           UMETA(DisplayName = "Error"),
    Critical        UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ValidationIssue
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    EBuild_ValidationResult Severity;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString FileName;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString Description;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 LineNumber;

    FBuild_ValidationIssue()
    {
        Severity = EBuild_ValidationResult::Success;
        LineNumber = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ValidationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FBuild_ValidationIssue> Issues;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 TotalHeaders;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 TotalImplementations;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 OrphanHeaders;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 DuplicateActors;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float ValidationTimeSeconds;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bCompilationSuccess;

    FBuild_ValidationReport()
    {
        TotalHeaders = 0;
        TotalImplementations = 0;
        OrphanHeaders = 0;
        DuplicateActors = 0;
        ValidationTimeSeconds = 0.0f;
        bCompilationSuccess = false;
    }
};

/**
 * Build Validation Manager - Sistema crítico de validação de build
 * Responsável por detectar headers órfãos, actores duplicados, e problemas de compilação
 * Implementa as regras críticas do brain memory para manter o projeto limpo
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuildValidationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuildValidationManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /**
     * Executa validação completa do projeto
     * Detecta headers órfãos, actores duplicados, problemas de compilação
     */
    UFUNCTION(BlueprintCallable, Category = "Build Validation", CallInEditor = true)
    FBuild_ValidationReport RunFullValidation();

    /**
     * Detecta headers sem implementação correspondente
     */
    UFUNCTION(BlueprintCallable, Category = "Build Validation", CallInEditor = true)
    TArray<FString> DetectOrphanHeaders();

    /**
     * Remove actores duplicados do MinPlayableMap
     */
    UFUNCTION(BlueprintCallable, Category = "Build Validation", CallInEditor = true)
    int32 CleanDuplicateActors();

    /**
     * Testa compilação via UnrealBuildTool
     */
    UFUNCTION(BlueprintCallable, Category = "Build Validation", CallInEditor = true)
    bool TestCompilation();

    /**
     * Valida distribuição de actores pelos 5 biomas
     */
    UFUNCTION(BlueprintCallable, Category = "Build Validation", CallInEditor = true)
    bool ValidateBiomeDistribution();

    /**
     * Gera relatório detalhado de validação
     */
    UFUNCTION(BlueprintCallable, Category = "Build Validation", CallInEditor = true)
    FString GenerateValidationReport(const FBuild_ValidationReport& Report);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    FBuild_ValidationReport LastValidationReport;

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    float LastValidationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    bool bAutoValidationEnabled;

private:
    // Validação de ficheiros
    void ValidateFileStructure(FBuild_ValidationReport& Report);
    void ValidateModuleDependencies(FBuild_ValidationReport& Report);
    void ValidateSharedTypes(FBuild_ValidationReport& Report);

    // Validação de actores
    void ValidateMapActors(FBuild_ValidationReport& Report);
    void ValidateLightingSetup(FBuild_ValidationReport& Report);
    void ValidateBiomeActors(FBuild_ValidationReport& Report);

    // Utilitários
    bool IsValidBiomePosition(const FVector& Position, EBiomeType BiomeType);
    FString GetModuleNameFromPath(const FString& FilePath);
    void AddValidationIssue(FBuild_ValidationReport& Report, EBuild_ValidationResult Severity, 
                           const FString& Module, const FString& File, const FString& Description, int32 Line = 0);
};