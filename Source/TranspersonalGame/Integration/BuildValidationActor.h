#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "BuildValidationActor.generated.h"

/**
 * Build Validation Actor
 * Actor dedicado para validação contínua do estado de integração do projecto.
 * Executa verificações automáticas e reporta problemas de integração.
 * Pode ser colocado no MinPlayableMap para monitorização em tempo real.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABuildValidationActor : public AActor
{
    GENERATED_BODY()
    
public:    
    ABuildValidationActor();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    /** Executar validação completa do projecto */
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void RunFullValidation();
    
    /** Validar apenas o estado do mapa actual */
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void ValidateCurrentMap();
    
    /** Validar compilação de módulos C++ */
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void ValidateModuleCompilation();
    
    /** Limpar problemas detectados automaticamente */
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void AutoFixDetectedIssues();
    
    /** Obter relatório de validação como string */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Build Validation")
    FString GetValidationReport() const;
    
    /** Verificar se o projecto está em estado saudável */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Build Validation")
    bool IsProjectHealthy() const;

protected:
    /** Validar actores de lighting no mundo */
    void CheckLightingActors();
    
    /** Validar actores de gameplay */
    void CheckGameplayActors();
    
    /** Validar configuração do GameMode */
    void CheckGameModeSetup();
    
    /** Detectar e reportar duplicados */
    void DetectDuplicates();
    
    /** Gerar relatório detalhado */
    void GenerateDetailedReport();

public:
    /** Se deve executar validação automática no Tick */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Settings")
    bool bAutoValidate;
    
    /** Intervalo entre validações automáticas (segundos) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Settings", meta = (ClampMin = "1.0"))
    float ValidationInterval;
    
    /** Se deve tentar corrigir problemas automaticamente */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Settings")
    bool bAutoFix;
    
    /** Se deve mostrar logs detalhados */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Settings")
    bool bVerboseLogging;

protected:
    /** Timer para validação automática */
    float ValidationTimer;
    
    /** Última vez que a validação foi executada */
    float LastValidationTime;

public:
    /** Número total de actores no mapa */
    UPROPERTY(BlueprintReadOnly, Category = "Validation Results")
    int32 TotalActors;
    
    /** Número de actores C++ detectados */
    UPROPERTY(BlueprintReadOnly, Category = "Validation Results")
    int32 CppActors;
    
    /** Número de actores de lighting */
    UPROPERTY(BlueprintReadOnly, Category = "Validation Results")
    int32 LightingActors;
    
    /** Número de duplicados detectados */
    UPROPERTY(BlueprintReadOnly, Category = "Validation Results")
    int32 DuplicatesFound;
    
    /** Número de erros críticos */
    UPROPERTY(BlueprintReadOnly, Category = "Validation Results")
    int32 CriticalErrors;
    
    /** Número de avisos */
    UPROPERTY(BlueprintReadOnly, Category = "Validation Results")
    int32 Warnings;
    
    /** Se a última validação passou */
    UPROPERTY(BlueprintReadOnly, Category = "Validation Results")
    bool bLastValidationPassed;
    
    /** Status da validação como string */
    UPROPERTY(BlueprintReadOnly, Category = "Validation Results")
    FString ValidationStatus;
    
    /** Lista de problemas detectados */
    UPROPERTY(BlueprintReadOnly, Category = "Validation Results")
    TArray<FString> DetectedIssues;
    
    /** Lista de classes C++ carregadas */
    UPROPERTY(BlueprintReadOnly, Category = "Validation Results")
    TArray<FString> LoadedCppClasses;
    
    /** Lista de classes C++ que falharam */
    UPROPERTY(BlueprintReadOnly, Category = "Validation Results")
    TArray<FString> FailedCppClasses;
};