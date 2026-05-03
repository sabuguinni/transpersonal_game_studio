#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "BuildIntegrationManager.generated.h"

/**
 * Build Integration Manager
 * Responsável por validar o estado de integração do projecto:
 * - Verificar se todos os módulos C++ compilam correctamente
 * - Validar o estado do MinPlayableMap
 * - Detectar duplicados de actores de lighting
 * - Gerar relatórios de integração
 * - Limpar problemas de integração automaticamente
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UBuildIntegrationManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UBuildIntegrationManager();

protected:
    virtual void BeginPlay() override;

public:
    /** Validar o estado actual do mapa */
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateMapState();
    
    /** Validar se os módulos C++ estão carregados correctamente */
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateCppModules();
    
    /** Gerar relatório completo de integração */
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void GenerateIntegrationReport();
    
    /** Verificar se a integração está saudável */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Integration")
    bool IsIntegrationHealthy() const;
    
    /** Limpar duplicados de lighting automaticamente */
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void CleanupDuplicateLighting();
    
    /** Obter status de integração como string */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Integration")
    FString GetIntegrationStatus() const;

protected:
    /** Validar actores de lighting no mundo */
    void ValidateLightingActors(UWorld* World);
    
    /** Validar actores C++ no mapa */
    void ValidateCppActors(const TArray<AActor*>& AllActors);
    
    /** Validar GameMode */
    void ValidateGameMode(UWorld* World);

public:
    /** Total de actores no mapa */
    UPROPERTY(BlueprintReadOnly, Category = "Integration Stats")
    int32 TotalActorsInMap;
    
    /** Número de actores C++ no mapa */
    UPROPERTY(BlueprintReadOnly, Category = "Integration Stats")
    int32 CppActorsCount;
    
    /** Número de actores de lighting */
    UPROPERTY(BlueprintReadOnly, Category = "Integration Stats")
    int32 LightingActorsCount;
    
    /** Se a compilação foi bem-sucedida */
    UPROPERTY(BlueprintReadOnly, Category = "Integration Stats")
    bool bCompilationSuccess;
    
    /** Se a validação do mapa passou */
    UPROPERTY(BlueprintReadOnly, Category = "Integration Stats")
    bool bMapValidationPassed;
    
    /** Lista de classes C++ carregadas com sucesso */
    UPROPERTY(BlueprintReadOnly, Category = "Integration Stats")
    TArray<FString> LoadedCppClasses;
    
    /** Lista de classes C++ que falharam ao carregar */
    UPROPERTY(BlueprintReadOnly, Category = "Integration Stats")
    TArray<FString> FailedCppClasses;
    
    /** Lista de duplicados de lighting encontrados */
    UPROPERTY(BlueprintReadOnly, Category = "Integration Stats")
    TArray<FString> DuplicateLightingActors;
    
    /** Lista de erros de validação */
    UPROPERTY(BlueprintReadOnly, Category = "Integration Stats")
    TArray<FString> ValidationErrors;
};