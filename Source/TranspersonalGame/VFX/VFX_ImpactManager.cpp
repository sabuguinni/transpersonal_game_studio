#include "VFX_ImpactManager.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Engine/Engine.h"

AVFX_ImpactManager::AVFX_ImpactManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Criar componente root
    RootMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RootMeshComponent"));
    RootComponent = RootMeshComponent;

    // Configurações padrão
    FootstepIntensity = 1.0f;
    BloodIntensity = 1.0f;
    DebrisSpread = 100.0f;
    bEnableFootstepVFX = true;
    bEnableBloodVFX = true;
    bEnableDebrisVFX = true;

    // Inicializar arrays
    ActiveFootstepVFX.Empty();
    ActiveBloodVFX.Empty();
    ActiveDebrisVFX.Empty();
}

void AVFX_ImpactManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeVFXSystems();
    SetupCleanupTimers();
    
    UE_LOG(LogTemp, Log, TEXT("VFX_ImpactManager: Inicializado com sucesso"));
}

void AVFX_ImpactManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Verificar se há VFX que precisam ser limpos
    // (Lógica de cleanup automático baseada em tempo)
}

void AVFX_ImpactManager::InitializeVFXSystems()
{
    // Tentar carregar sistemas Niagara do projeto
    // Se não existirem, usar sistemas padrão do Engine
    
    if (!FootstepDustSystem)
    {
        // Tentar carregar sistema personalizado primeiro
        FootstepDustSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/NS_FootstepDust"));
        
        if (!FootstepDustSystem)
        {
            // Fallback para sistema do Engine se disponível
            UE_LOG(LogTemp, Warning, TEXT("VFX_ImpactManager: Sistema de footstep personalizado não encontrado"));
        }
    }
    
    if (!BloodSplatterSystem)
    {
        BloodSplatterSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/NS_BloodSplatter"));
        
        if (!BloodSplatterSystem)
        {
            UE_LOG(LogTemp, Warning, TEXT("VFX_ImpactManager: Sistema de blood splatter não encontrado"));
        }
    }
    
    if (!DebrisSystem)
    {
        DebrisSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/NS_Debris"));
        
        if (!DebrisSystem)
        {
            UE_LOG(LogTemp, Warning, TEXT("VFX_ImpactManager: Sistema de debris não encontrado"));
        }
    }
    
    if (!RockImpactSystem)
    {
        RockImpactSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/NS_RockImpact"));
        
        if (!RockImpactSystem)
        {
            UE_LOG(LogTemp, Warning, TEXT("VFX_ImpactManager: Sistema de rock impact não encontrado"));
        }
    }
}

void AVFX_ImpactManager::SetupCleanupTimers()
{
    // Configurar timers para limpeza automática de VFX antigos
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            FootstepCleanupTimer,
            this,
            &AVFX_ImpactManager::CleanupOldVFX,
            5.0f,  // Cleanup a cada 5 segundos
            true   // Repetir
        );
    }
}

void AVFX_ImpactManager::TriggerFootstepVFX(FVector Location, float DinosaurSize)
{
    if (!bEnableFootstepVFX || !FootstepDustSystem)
    {
        return;
    }
    
    // Ajustar intensidade baseada no tamanho do dinossauro
    float ScaledIntensity = FootstepIntensity * DinosaurSize;
    
    // Spawnar efeito VFX
    UNiagaraComponent* FootstepVFX = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        FootstepDustSystem,
        Location,
        FRotator::ZeroRotator,
        FVector(ScaledIntensity),  // Scale baseado na intensidade
        true,  // Auto destroy
        true,  // Auto activate
        ENCPoolMethod::None,
        true   // Pre cull check
    );
    
    if (FootstepVFX)
    {
        // Configurar parâmetros específicos do footstep
        FootstepVFX->SetFloatParameter(TEXT("DustAmount"), ScaledIntensity);
        FootstepVFX->SetFloatParameter(TEXT("ParticleSize"), DinosaurSize * 0.5f);
        
        ActiveFootstepVFX.Add(FootstepVFX);
        
        UE_LOG(LogTemp, Log, TEXT("VFX_ImpactManager: Footstep VFX triggered at %s"), *Location.ToString());
    }
}

void AVFX_ImpactManager::TriggerBloodSplatterVFX(FVector Location, FVector ImpactDirection)
{
    if (!bEnableBloodVFX || !BloodSplatterSystem)
    {
        return;
    }
    
    // Calcular rotação baseada na direção do impacto
    FRotator ImpactRotation = ImpactDirection.Rotation();
    
    UNiagaraComponent* BloodVFX = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        BloodSplatterSystem,
        Location,
        ImpactRotation,
        FVector(BloodIntensity),
        true,  // Auto destroy
        true,  // Auto activate
        ENCPoolMethod::None,
        true   // Pre cull check
    );
    
    if (BloodVFX)
    {
        // Configurar parâmetros do blood splatter
        BloodVFX->SetVectorParameter(TEXT("ImpactDirection"), ImpactDirection);
        BloodVFX->SetFloatParameter(TEXT("SplatterIntensity"), BloodIntensity);
        
        ActiveBloodVFX.Add(BloodVFX);
        
        UE_LOG(LogTemp, Log, TEXT("VFX_ImpactManager: Blood splatter VFX triggered"));
    }
}

void AVFX_ImpactManager::TriggerDebrisVFX(FVector Location, FVector ExplosionDirection, float Force)
{
    if (!bEnableDebrisVFX || !DebrisSystem)
    {
        return;
    }
    
    FRotator ExplosionRotation = ExplosionDirection.Rotation();
    
    UNiagaraComponent* DebrisVFX = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        DebrisSystem,
        Location,
        ExplosionRotation,
        FVector(Force * 0.01f),  // Scale baseado na força
        true,  // Auto destroy
        true,  // Auto activate
        ENCPoolMethod::None,
        true   // Pre cull check
    );
    
    if (DebrisVFX)
    {
        // Configurar parâmetros do debris
        DebrisVFX->SetVectorParameter(TEXT("ExplosionDirection"), ExplosionDirection);
        DebrisVFX->SetFloatParameter(TEXT("ExplosionForce"), Force);
        DebrisVFX->SetFloatParameter(TEXT("DebrisSpread"), DebrisSpread);
        
        ActiveDebrisVFX.Add(DebrisVFX);
        
        UE_LOG(LogTemp, Log, TEXT("VFX_ImpactManager: Debris VFX triggered with force %f"), Force);
    }
}

void AVFX_ImpactManager::TriggerRockImpactVFX(FVector Location, float ImpactForce)
{
    if (!RockImpactSystem)
    {
        return;
    }
    
    UNiagaraComponent* RockVFX = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        RockImpactSystem,
        Location,
        FRotator::ZeroRotator,
        FVector(ImpactForce * 0.01f),
        true,  // Auto destroy
        true,  // Auto activate
        ENCPoolMethod::None,
        true   // Pre cull check
    );
    
    if (RockVFX)
    {
        RockVFX->SetFloatParameter(TEXT("ImpactForce"), ImpactForce);
        
        UE_LOG(LogTemp, Log, TEXT("VFX_ImpactManager: Rock impact VFX triggered"));
    }
}

void AVFX_ImpactManager::SetVFXIntensity(float NewIntensity)
{
    FootstepIntensity = FMath::Clamp(NewIntensity, 0.1f, 5.0f);
    BloodIntensity = FootstepIntensity;
    
    UE_LOG(LogTemp, Log, TEXT("VFX_ImpactManager: Intensidade VFX alterada para %f"), NewIntensity);
}

void AVFX_ImpactManager::EnableAllVFX(bool bEnable)
{
    bEnableFootstepVFX = bEnable;
    bEnableBloodVFX = bEnable;
    bEnableDebrisVFX = bEnable;
    
    UE_LOG(LogTemp, Log, TEXT("VFX_ImpactManager: Todos os VFX %s"), bEnable ? TEXT("activados") : TEXT("desactivados"));
}

void AVFX_ImpactManager::CleanupOldVFX()
{
    CleanupVFXArray(ActiveFootstepVFX);
    CleanupVFXArray(ActiveBloodVFX);
    CleanupVFXArray(ActiveDebrisVFX);
}

UNiagaraComponent* AVFX_ImpactManager::SpawnVFXAtLocation(UNiagaraSystem* System, FVector Location, FRotator Rotation)
{
    if (!System || !GetWorld())
    {
        return nullptr;
    }
    
    return UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        System,
        Location,
        Rotation,
        FVector::OneVector,
        true,  // Auto destroy
        true,  // Auto activate
        ENCPoolMethod::None,
        true   // Pre cull check
    );
}

void AVFX_ImpactManager::CleanupVFXArray(TArray<UNiagaraComponent*>& VFXArray)
{
    // Remover componentes que já não são válidos ou terminaram
    VFXArray.RemoveAll([](UNiagaraComponent* Component)
    {
        return !IsValid(Component) || !Component->IsActive();
    });
}