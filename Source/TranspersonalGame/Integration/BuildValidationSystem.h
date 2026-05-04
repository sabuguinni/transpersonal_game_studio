#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "BuildValidationSystem.generated.h"

UENUM(BlueprintType)
enum class EBuild_ValidationResult : uint8
{
    Success         UMETA(DisplayName = "Success"),
    Warning         UMETA(DisplayName = "Warning"),
    Error           UMETA(DisplayName = "Error"),
    CriticalError   UMETA(DisplayName = "Critical Error")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ValidationIssue
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    EBuild_ValidationResult Severity;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString Category;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString Description;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString ActorName;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FVector Location;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float Timestamp;

    FBuild_ValidationIssue()
    {
        Severity = EBuild_ValidationResult::Success;
        Category = TEXT("");
        Description = TEXT("");
        ActorName = TEXT("");
        Location = FVector::ZeroVector;
        Timestamp = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ValidationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FBuild_ValidationIssue> Issues;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 TotalActors;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 ErrorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 WarningCount;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float ValidationDuration;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString ReportTimestamp;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bHasCriticalErrors;

    FBuild_ValidationReport()
    {
        TotalActors = 0;
        ErrorCount = 0;
        WarningCount = 0;
        ValidationDuration = 0.0f;
        ReportTimestamp = TEXT("");
        bHasCriticalErrors = false;
    }
};

/**
 * Sistema de validação de build que verifica a integridade do mundo e dos actores
 * Detecta problemas de integração, duplicados críticos e dependências em falta
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABuild_ValidationSystem : public AActor
{
    GENERATED_BODY()

public:
    ABuild_ValidationSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Executar validação completa do mundo
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    FBuild_ValidationReport RunFullValidation();

    // Validar tipos de actores críticos
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void ValidateCriticalActorTypes();

    // Validar dependências entre sistemas
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void ValidateSystemDependencies();

    // Validar configuração de lighting
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void ValidateLightingSetup();

    // Validar distribuição de actores por biomas
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void ValidateBiomeDistribution();

    // Validar performance e contagem de actores
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void ValidatePerformanceMetrics();

    // Obter último relatório de validação
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    FBuild_ValidationReport GetLastValidationReport() const;

    // Verificar se existem erros críticos
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    bool HasCriticalErrors() const;

    // Gerar relatório detalhado em texto
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    FString GenerateDetailedReport() const;

protected:
    // Configuração de validação
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Config")
    bool bAutoValidateOnTick;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Config")
    float ValidationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Config")
    int32 MaxActorsPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Config")
    int32 MaxTotalActors;

    // Tipos críticos que devem ter apenas 1 instância
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Config")
    TArray<FString> SingletonActorTypes;

    // Tipos obrigatórios que devem existir
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Config")
    TArray<FString> RequiredActorTypes;

    // Relatório atual
    UPROPERTY(BlueprintReadOnly, Category = "Validation State")
    FBuild_ValidationReport CurrentReport;

    // Estado interno
    UPROPERTY()
    float LastValidationTime;

    UPROPERTY()
    TMap<FString, int32> ActorTypeCountCache;

private:
    // Métodos auxiliares de validação
    void AddValidationIssue(EBuild_ValidationResult Severity, const FString& Category, 
                           const FString& Description, const FString& ActorName = TEXT(""), 
                           const FVector& Location = FVector::ZeroVector);

    void ValidateActorCount(const FString& ActorType, int32 ExpectedCount, bool bExactMatch = true);
    void ValidateActorExists(const FString& ActorType);
    void ValidateNoDuplicates(const FString& ActorType);
    void ValidateBiomeCoordinates();
    void ValidateActorDistribution();
    
    bool IsLocationInBiome(const FVector& Location, EBiomeType BiomeType) const;
    FString GetBiomeNameFromType(EBiomeType BiomeType) const;
    EBiomeType GetBiomeFromLocation(const FVector& Location) const;
};